package com.gsma.services.rcs.contact;

import com.gsma.services.rcs.contact.JoynContact;
import com.gsma.services.rcs.contact.RcsContact;
import com.gsma.services.rcs.contact.ContactId;

/**
 * Contacts service API
 */
interface IContactService {
	JoynContact getJoynContact(String contactId);

	List<JoynContact> getJoynContacts();

	List<JoynContact> getJoynContactsOnline();

	List<JoynContact> getJoynContactsSupporting(in String tag);
	
	int getServiceVersion();
	
	List<String> getImBlockedContactsFromLocal();
	
	boolean isImBlockedForContact(String contact);
	
	List<String> getImBlockedContacts();
	
	String getTimeStampForBlockedContact(in String contact);
	
	void setImBlockedForContact(in String contact, in boolean flag);
	
	void setFtBlockedForContact(in String contact, in boolean flag);
	
	boolean isRcsValidNumber(in String number);
	
	int getRegistrationState(in String contact);
	
	void loadImBlockedContactsToLocal();
	
	RcsContact getRcsContact(in ContactId contact);

    List<RcsContact> getRcsContacts();

    List<RcsContact> getRcsContactsOnline();

    List<RcsContact> getRcsContactsSupporting(in String tag);
    
    void blockContact(in ContactId contact);

    void unblockContact(in ContactId contact);
	
}
