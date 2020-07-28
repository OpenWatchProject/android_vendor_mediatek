/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.mail.vip.activity;

import java.util.ArrayList;

import com.android.mail.R;
import com.android.emailcommon.mail.Address;
import com.android.emailcommon.utility.AsyncTask;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.TextUtilities;
import com.android.emailcommon.utility.Utility;
import com.android.mtkex.chips.AccountSpecifier;
import com.android.mtkex.chips.BaseRecipientAdapter;
import com.mediatek.mail.ui.utils.ChipsAddressTextView;
import com.mediatek.mail.vip.VipMember;
import com.mediatek.mail.vip.activity.VipListAdapter.AvatarInfo;

import android.app.Activity;
import android.app.ListFragment;
import android.app.LoaderManager;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Loader;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.provider.ContactsContract;
import android.text.InputFilter;
import android.text.util.Rfc822Tokenizer;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AutoCompleteTextView.Validator;
import android.widget.ListView;


/**
 * M: The Fragment was used for contain of all the VIP feature's UI components.
 *
 */
public class VipListFragment extends ListFragment implements VipListAdapter.AvatarLoaderCallback {
    // UI Support
    private Activity mActivity;
    private ListView mListView;
    private VipListAdapter mListAdapter;
    private VipAddressTextView mSearchVipView;
    private AccountSpecifier mAddressAdapter;
    private View mSearchContent;
    /** Arbitrary number for use with the loader manager */
    private static final int VIP_LOADER_ID = 1;
    /** Argument name(s) */
    private static final String ARG_ACCOUNT_ID = "accountId";
    public static final int EDITVIEW_MAX_LENGTH = 256;
    private Long mImmutableAccountId;
    private int mVipNumber;
    private String mNewVipAddress = null;

    private Callback mCallback;
    private ContentResolver mContentResolver;
    private ContactsProviderObserver mContactsObserver;

    /**
     * Callback interface that owning activities must implement
     */
    public interface Callback {
        /**
         * Called when the vip numbers changed.
         */
        public void onVipMemberChanged(int vipNumber);
    }

    public static VipListFragment newInstance(Long accountID) {
        VipListFragment f = new VipListFragment();
        Bundle bundle = new Bundle();
        bundle.putLong(ARG_ACCOUNT_ID, accountID);
        f.setArguments(bundle);
        return f;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mActivity = getActivity();
        mListAdapter = new VipListAdapter(mActivity, this);
        setListAdapter(mListAdapter);
        mImmutableAccountId = getArguments().getLong(ARG_ACCOUNT_ID);
        mCallback = (Callback) mActivity;

        ListPhotoManager.getInstance(mActivity).refreshCache();

        mContentResolver = mActivity.getContentResolver();
        ///M: set a handler for let the onChange() to be execute on UI thread to avoid
        //  concrete withUI refresh.
        mContactsObserver = new ContactsProviderObserver(new Handler());
        mContentResolver.registerContentObserver(ContactsContract.Data.CONTENT_URI, true, mContactsObserver);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mListView = getListView();
        mListView.setOnScrollListener(new OnScrollListener() {
            @Override
            public void onScrollStateChanged(AbsListView view, int scrollState) {
                if (scrollState == OnScrollListener.SCROLL_STATE_FLING) {
                    mListAdapter.pauseAvatarLoading(true);
                } else {
                    mListAdapter.pauseAvatarLoading(false);
                }
            }
            @Override
            public void onScroll(AbsListView view, int firstVisibleItem,
                    int visibleItemCount, int totalItemCount) {
            }
        });
        setListShown(false);
        startLoading();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mListAdapter.onViewDestroy();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.email_vip_fragment, container, false);
        mSearchVipView = (VipAddressTextView) view.findViewById(R.id.search_vip);
        mSearchVipView.setTargetFragment(this);
        mAddressAdapter = new RecipientAdapter(getActivity(),
                (VipAddressTextView) mSearchVipView);
        InputFilter[] recipientFilters = new InputFilter[] { ChipsAddressTextView.RECIPIENT_FILTER };
        mSearchVipView.setFilters(recipientFilters);
        mSearchVipView.setValidator(new AddressValidator());
        mSearchVipView.setAdapter((RecipientAdapter) mAddressAdapter);
        mSearchVipView.setTokenizer(new Rfc822Tokenizer());
        mSearchVipView.setGalSearchDelayer();

        mSearchContent = view.findViewById(R.id.to_content);
        mSearchContent.setVisibility(View.INVISIBLE);

        TextUtilities.setupLengthFilter(mSearchVipView, getActivity(),
                EDITVIEW_MAX_LENGTH, getActivity().getString(R.string.not_add_more_text));
        return view;
    }

    /**
     * Starts the loader.
     *
     */
    private void startLoading() {
        final LoaderManager lm = getLoaderManager();
        // Update the Vip list action bar title.
        if (mCallback != null) {
            mCallback.onVipMemberChanged(mVipNumber);
        }
        lm.initLoader(VIP_LOADER_ID, null, new VipListLoaderCallbacks());
    }

    private class AddressValidator implements Validator {
        public CharSequence fixText(CharSequence invalidText) {
            return "";
        }

        public boolean isValid(CharSequence text) {
            return Address.parse(text.toString()).length > 0;
        }
    }

    public void onAddVip(final Address[] addresses) {
        EmailAsyncTask.runAsyncParallel(new Runnable() {
            @Override
            public void run() {
                VipListFragment.this.saveAsVips(addresses);
            }

        });
    }

    private void saveAsVips(Address[] addresses) {
        ArrayList<Address> addressList = new ArrayList<Address>();
        for (Address addr : addresses) {
            addressList.add(addr);
        }
        if (addressList.size() > 0) {
            mNewVipAddress = addressList.get(addressList.size() - 1).getAddress();
        }
        VipMember.addVIPs(mActivity, mImmutableAccountId, addressList, new VipMember.AddVipsCallback() {
            @Override
            public void tryToAddDuplicateVip() {
                Utility.showToast(mActivity, R.string.not_add_duplicate_vip);
            }
            @Override
            public void addVipOverMax() {
                Utility.showToast(mActivity, R.string.can_not_add_vip_over_99);
            }
        });
    }

    private class VipListLoaderCallbacks implements LoaderCallbacks<Cursor> {

        @Override
        public Loader<Cursor> onCreateLoader(int id, Bundle args) {
            return VipListAdapter.createVipContentLoader(getActivity(), mImmutableAccountId);
        }

        @Override
        public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
            // Always swap out the cursor so we don't hold a reference to a stale one.
            mListAdapter.swapCursor(data);
            setListShown(true);
            mSearchContent.setVisibility(View.VISIBLE);

            // Update the Vip list action bar title.
            if (mCallback != null) {
                mVipNumber = data.getCount();
                mCallback.onVipMemberChanged(mVipNumber);
            }

            // Scroll to the new added vip item.
            if (mNewVipAddress != null) {
                int position = mListAdapter.getPosition(mNewVipAddress);
                if (position != -1) {
                    mListView.setSelection(position);
                }
                mNewVipAddress = null;
            }
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {
            mListAdapter.swapCursor(null);
        }
    }

    private class RecipientAdapter extends BaseRecipientAdapter {
        public RecipientAdapter(Context context, VipAddressTextView list) {
            super(context);
        }

        /**
         * Set the account when known. Causes the search to prioritize contacts from
         * that account.
         */
        @Override
        public void setAccount(android.accounts.Account account) {
            if (account != null) {
                // TODO: figure out how to infer the contacts account
                // type from the email account
                super.setAccount(new android.accounts.Account(account.name, "unknown"));
            }
        }
    }

    @Override
    public void onAvatarNameChanged(final AvatarInfo avatarInfo) {
        // Avoid block the mainthread, start an runnable to update vip name in database.
        new EmailAsyncTask<Void, Void, Void>(null) {
            @Override
            protected Void doInBackground(Void... params) {
                VipMember.updateVipDisplayName(mActivity, mImmutableAccountId, avatarInfo.mAddress,
                        avatarInfo.mDisplayName);
                return null;
            }
        } .executeExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    @Override
    public void onDestroy() {
        mContentResolver.unregisterContentObserver(mContactsObserver);
        super.onDestroy();
    }

    /**
     * Observer using for monitor contacts data changes.
     */
    private class ContactsProviderObserver extends ContentObserver {

        public ContactsProviderObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            Cursor vipItemsCursor = mListAdapter.getCursor();
            if (vipItemsCursor == null || vipItemsCursor.isClosed()) {
                return;
            }

            ListPhotoManager.getInstance(mActivity).refreshCache();
            // Update current UI listview items.
            vipItemsCursor.moveToPosition(-1);
            while (vipItemsCursor.moveToNext()) {
                mListAdapter.updateAvatar(vipItemsCursor.getString(VipMember.EMAIL_ADDRESS_COLUMN));
            }
        }
    }
}
