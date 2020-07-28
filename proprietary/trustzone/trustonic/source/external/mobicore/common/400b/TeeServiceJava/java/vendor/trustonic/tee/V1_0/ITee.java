/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package vendor.trustonic.tee.V1_0;

public interface ITee extends android.hidl.base.V1_0.IBase {
    public static final String kInterfaceName = "vendor.trustonic.tee@1.0::ITee";

    /* package private */ static ITee asInterface(android.os.IHwBinder binder) {
        if (binder == null) {
            return null;
        }

        android.os.IHwInterface iface =
                binder.queryLocalInterface(kInterfaceName);

        if ((iface != null) && (iface instanceof ITee)) {
            return (ITee)iface;
        }

        ITee proxy = new ITee.Proxy(binder);

        try {
            for (String descriptor : proxy.interfaceChain()) {
                if (descriptor.equals(kInterfaceName)) {
                    return proxy;
                }
            }
        } catch (android.os.RemoteException e) {
        }

        return null;
    }

    public static ITee castFrom(android.os.IHwInterface iface) {
        return (iface == null) ? null : ITee.asInterface(iface.asBinder());
    }

    @Override
    public android.os.IHwBinder asBinder();

    public static ITee getService(String serviceName, boolean retry) throws android.os.RemoteException {
        return ITee.asInterface(android.os.HwBinder.getService("vendor.trustonic.tee@1.0::ITee", serviceName, retry));
    }

    public static ITee getService(boolean retry) throws android.os.RemoteException {
        return getService("default", retry);
    }

    public static ITee getService(String serviceName) throws android.os.RemoteException {
        return ITee.asInterface(android.os.HwBinder.getService("vendor.trustonic.tee@1.0::ITee", serviceName));
    }

    public static ITee getService() throws android.os.RemoteException {
        return getService("default");
    }

    public static final class LoginType {
        public static final int TEE_PROXY_LOGIN_PUBLIC = 0;
        public static final int TEE_PROXY_LOGIN_USER = 1;
        public static final int TEE_PROXY_LOGIN_GROUP = 2;
        public static final int TEE_PROXY_LOGIN_APPLICATION = 4;
        public static final int TEE_PROXY_LOGIN_USER_APPLICATION = 5;
        public static final int TEE_PROXY_LOGIN_GROUP_APPLICATION = 6;
        public static final String toString(int o) {
            if (o == TEE_PROXY_LOGIN_PUBLIC) {
                return "TEE_PROXY_LOGIN_PUBLIC";
            }
            if (o == TEE_PROXY_LOGIN_USER) {
                return "TEE_PROXY_LOGIN_USER";
            }
            if (o == TEE_PROXY_LOGIN_GROUP) {
                return "TEE_PROXY_LOGIN_GROUP";
            }
            if (o == TEE_PROXY_LOGIN_APPLICATION) {
                return "TEE_PROXY_LOGIN_APPLICATION";
            }
            if (o == TEE_PROXY_LOGIN_USER_APPLICATION) {
                return "TEE_PROXY_LOGIN_USER_APPLICATION";
            }
            if (o == TEE_PROXY_LOGIN_GROUP_APPLICATION) {
                return "TEE_PROXY_LOGIN_GROUP_APPLICATION";
            }
            return "0x" + Integer.toHexString(o);
        }

        public static final String dumpBitfield(int o) {
            java.util.ArrayList<String> list = new java.util.ArrayList<>();
            int flipped = 0;
            list.add("TEE_PROXY_LOGIN_PUBLIC"); // TEE_PROXY_LOGIN_PUBLIC == 0
            if ((o & TEE_PROXY_LOGIN_USER) == TEE_PROXY_LOGIN_USER) {
                list.add("TEE_PROXY_LOGIN_USER");
                flipped |= TEE_PROXY_LOGIN_USER;
            }
            if ((o & TEE_PROXY_LOGIN_GROUP) == TEE_PROXY_LOGIN_GROUP) {
                list.add("TEE_PROXY_LOGIN_GROUP");
                flipped |= TEE_PROXY_LOGIN_GROUP;
            }
            if ((o & TEE_PROXY_LOGIN_APPLICATION) == TEE_PROXY_LOGIN_APPLICATION) {
                list.add("TEE_PROXY_LOGIN_APPLICATION");
                flipped |= TEE_PROXY_LOGIN_APPLICATION;
            }
            if ((o & TEE_PROXY_LOGIN_USER_APPLICATION) == TEE_PROXY_LOGIN_USER_APPLICATION) {
                list.add("TEE_PROXY_LOGIN_USER_APPLICATION");
                flipped |= TEE_PROXY_LOGIN_USER_APPLICATION;
            }
            if ((o & TEE_PROXY_LOGIN_GROUP_APPLICATION) == TEE_PROXY_LOGIN_GROUP_APPLICATION) {
                list.add("TEE_PROXY_LOGIN_GROUP_APPLICATION");
                flipped |= TEE_PROXY_LOGIN_GROUP_APPLICATION;
            }
            if (o != flipped) {
                list.add("0x" + Integer.toHexString(o & (~flipped)));
            }
            return String.join(" | ", list);
        }

    };

    public final static class GpSharedMemory {
        public long reference;
        public long buffer;
        public long size;
        public int flags;

        @Override
        public final boolean equals(Object otherObject) {
            if (this == otherObject) {
                return true;
            }
            if (otherObject == null) {
                return false;
            }
            if (otherObject.getClass() != vendor.trustonic.tee.V1_0.ITee.GpSharedMemory.class) {
                return false;
            }
            vendor.trustonic.tee.V1_0.ITee.GpSharedMemory other = (vendor.trustonic.tee.V1_0.ITee.GpSharedMemory)otherObject;
            if (this.reference != other.reference) {
                return false;
            }
            if (this.buffer != other.buffer) {
                return false;
            }
            if (this.size != other.size) {
                return false;
            }
            if (this.flags != other.flags) {
                return false;
            }
            return true;
        }

        @Override
        public final int hashCode() {
            return java.util.Objects.hash(
                    android.os.HidlSupport.deepHashCode(this.reference),
                    android.os.HidlSupport.deepHashCode(this.buffer),
                    android.os.HidlSupport.deepHashCode(this.size),
                    android.os.HidlSupport.deepHashCode(this.flags));
        }

        @Override
        public final String toString() {
            java.lang.StringBuilder builder = new java.lang.StringBuilder();
            builder.append("{");
            builder.append(".reference = ");
            builder.append(this.reference);
            builder.append(", .buffer = ");
            builder.append(this.buffer);
            builder.append(", .size = ");
            builder.append(this.size);
            builder.append(", .flags = ");
            builder.append(this.flags);
            builder.append("}");
            return builder.toString();
        }

        public final void readFromParcel(android.os.HwParcel parcel) {
            android.os.HwBlob blob = parcel.readBuffer(32/* size */);
            readEmbeddedFromParcel(parcel, blob, 0 /* parentOffset */);
        }

        public static final java.util.ArrayList<GpSharedMemory> readVectorFromParcel(android.os.HwParcel parcel) {
            java.util.ArrayList<GpSharedMemory> _hidl_vec = new java.util.ArrayList();
            android.os.HwBlob _hidl_blob = parcel.readBuffer(16 /* sizeof hidl_vec<T> */);

            {
                int _hidl_vec_size = _hidl_blob.getInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */);
                android.os.HwBlob childBlob = parcel.readEmbeddedBuffer(
                        _hidl_vec_size * 32,_hidl_blob.handle(),
                        0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */,true /* nullable */);

                _hidl_vec.clear();
                for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                    final vendor.trustonic.tee.V1_0.ITee.GpSharedMemory _hidl_vec_element = new vendor.trustonic.tee.V1_0.ITee.GpSharedMemory();
                    _hidl_vec_element.readEmbeddedFromParcel(parcel, childBlob, _hidl_index_0 * 32);
                    _hidl_vec.add(_hidl_vec_element);
                }
            }

            return _hidl_vec;
        }

        public final void readEmbeddedFromParcel(
                android.os.HwParcel parcel, android.os.HwBlob _hidl_blob, long _hidl_offset) {
            reference = _hidl_blob.getInt64(_hidl_offset + 0);
            buffer = _hidl_blob.getInt64(_hidl_offset + 8);
            size = _hidl_blob.getInt64(_hidl_offset + 16);
            flags = _hidl_blob.getInt32(_hidl_offset + 24);
        }

        public final void writeToParcel(android.os.HwParcel parcel) {
            android.os.HwBlob _hidl_blob = new android.os.HwBlob(32 /* size */);
            writeEmbeddedToBlob(_hidl_blob, 0 /* parentOffset */);
            parcel.writeBuffer(_hidl_blob);
        }

        public static final void writeVectorToParcel(
                android.os.HwParcel parcel, java.util.ArrayList<GpSharedMemory> _hidl_vec) {
            android.os.HwBlob _hidl_blob = new android.os.HwBlob(16 /* sizeof(hidl_vec<T>) */);
            {
                int _hidl_vec_size = _hidl_vec.size();
                _hidl_blob.putInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */, _hidl_vec_size);
                _hidl_blob.putBool(0 + 12 /* offsetof(hidl_vec<T>, mOwnsBuffer) */, false);
                android.os.HwBlob childBlob = new android.os.HwBlob((int)(_hidl_vec_size * 32));
                for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                    _hidl_vec.get(_hidl_index_0).writeEmbeddedToBlob(childBlob, _hidl_index_0 * 32);
                }
                _hidl_blob.putBlob(0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */, childBlob);
            }

            parcel.writeBuffer(_hidl_blob);
        }

        public final void writeEmbeddedToBlob(
                android.os.HwBlob _hidl_blob, long _hidl_offset) {
            _hidl_blob.putInt64(_hidl_offset + 0, reference);
            _hidl_blob.putInt64(_hidl_offset + 8, buffer);
            _hidl_blob.putInt64(_hidl_offset + 16, size);
            _hidl_blob.putInt32(_hidl_offset + 24, flags);
        }
    };

    public final static class Operation {
        public final static class Param {
            public int type;
            public int value_a;
            public int value_b;
            public boolean has_buffer;
            public long reference;
            public long buffer;
            public long size;
            public int flags;
            public long window_offset;
            public long window_size;

            @Override
            public final boolean equals(Object otherObject) {
                if (this == otherObject) {
                    return true;
                }
                if (otherObject == null) {
                    return false;
                }
                if (otherObject.getClass() != vendor.trustonic.tee.V1_0.ITee.Operation.Param.class) {
                    return false;
                }
                vendor.trustonic.tee.V1_0.ITee.Operation.Param other = (vendor.trustonic.tee.V1_0.ITee.Operation.Param)otherObject;
                if (this.type != other.type) {
                    return false;
                }
                if (this.value_a != other.value_a) {
                    return false;
                }
                if (this.value_b != other.value_b) {
                    return false;
                }
                if (this.has_buffer != other.has_buffer) {
                    return false;
                }
                if (this.reference != other.reference) {
                    return false;
                }
                if (this.buffer != other.buffer) {
                    return false;
                }
                if (this.size != other.size) {
                    return false;
                }
                if (this.flags != other.flags) {
                    return false;
                }
                if (this.window_offset != other.window_offset) {
                    return false;
                }
                if (this.window_size != other.window_size) {
                    return false;
                }
                return true;
            }

            @Override
            public final int hashCode() {
                return java.util.Objects.hash(
                        android.os.HidlSupport.deepHashCode(this.type),
                        android.os.HidlSupport.deepHashCode(this.value_a),
                        android.os.HidlSupport.deepHashCode(this.value_b),
                        android.os.HidlSupport.deepHashCode(this.has_buffer),
                        android.os.HidlSupport.deepHashCode(this.reference),
                        android.os.HidlSupport.deepHashCode(this.buffer),
                        android.os.HidlSupport.deepHashCode(this.size),
                        android.os.HidlSupport.deepHashCode(this.flags),
                        android.os.HidlSupport.deepHashCode(this.window_offset),
                        android.os.HidlSupport.deepHashCode(this.window_size));
            }

            @Override
            public final String toString() {
                java.lang.StringBuilder builder = new java.lang.StringBuilder();
                builder.append("{");
                builder.append(".type = ");
                builder.append(this.type);
                builder.append(", .value_a = ");
                builder.append(this.value_a);
                builder.append(", .value_b = ");
                builder.append(this.value_b);
                builder.append(", .has_buffer = ");
                builder.append(this.has_buffer);
                builder.append(", .reference = ");
                builder.append(this.reference);
                builder.append(", .buffer = ");
                builder.append(this.buffer);
                builder.append(", .size = ");
                builder.append(this.size);
                builder.append(", .flags = ");
                builder.append(this.flags);
                builder.append(", .window_offset = ");
                builder.append(this.window_offset);
                builder.append(", .window_size = ");
                builder.append(this.window_size);
                builder.append("}");
                return builder.toString();
            }

            public final void readFromParcel(android.os.HwParcel parcel) {
                android.os.HwBlob blob = parcel.readBuffer(64/* size */);
                readEmbeddedFromParcel(parcel, blob, 0 /* parentOffset */);
            }

            public static final java.util.ArrayList<Param> readVectorFromParcel(android.os.HwParcel parcel) {
                java.util.ArrayList<Param> _hidl_vec = new java.util.ArrayList();
                android.os.HwBlob _hidl_blob = parcel.readBuffer(16 /* sizeof hidl_vec<T> */);

                {
                    int _hidl_vec_size = _hidl_blob.getInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */);
                    android.os.HwBlob childBlob = parcel.readEmbeddedBuffer(
                            _hidl_vec_size * 64,_hidl_blob.handle(),
                            0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */,true /* nullable */);

                    _hidl_vec.clear();
                    for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                        final vendor.trustonic.tee.V1_0.ITee.Operation.Param _hidl_vec_element = new vendor.trustonic.tee.V1_0.ITee.Operation.Param();
                        _hidl_vec_element.readEmbeddedFromParcel(parcel, childBlob, _hidl_index_0 * 64);
                        _hidl_vec.add(_hidl_vec_element);
                    }
                }

                return _hidl_vec;
            }

            public final void readEmbeddedFromParcel(
                    android.os.HwParcel parcel, android.os.HwBlob _hidl_blob, long _hidl_offset) {
                type = _hidl_blob.getInt32(_hidl_offset + 0);
                value_a = _hidl_blob.getInt32(_hidl_offset + 4);
                value_b = _hidl_blob.getInt32(_hidl_offset + 8);
                has_buffer = _hidl_blob.getBool(_hidl_offset + 12);
                reference = _hidl_blob.getInt64(_hidl_offset + 16);
                buffer = _hidl_blob.getInt64(_hidl_offset + 24);
                size = _hidl_blob.getInt64(_hidl_offset + 32);
                flags = _hidl_blob.getInt32(_hidl_offset + 40);
                window_offset = _hidl_blob.getInt64(_hidl_offset + 48);
                window_size = _hidl_blob.getInt64(_hidl_offset + 56);
            }

            public final void writeToParcel(android.os.HwParcel parcel) {
                android.os.HwBlob _hidl_blob = new android.os.HwBlob(64 /* size */);
                writeEmbeddedToBlob(_hidl_blob, 0 /* parentOffset */);
                parcel.writeBuffer(_hidl_blob);
            }

            public static final void writeVectorToParcel(
                    android.os.HwParcel parcel, java.util.ArrayList<Param> _hidl_vec) {
                android.os.HwBlob _hidl_blob = new android.os.HwBlob(16 /* sizeof(hidl_vec<T>) */);
                {
                    int _hidl_vec_size = _hidl_vec.size();
                    _hidl_blob.putInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */, _hidl_vec_size);
                    _hidl_blob.putBool(0 + 12 /* offsetof(hidl_vec<T>, mOwnsBuffer) */, false);
                    android.os.HwBlob childBlob = new android.os.HwBlob((int)(_hidl_vec_size * 64));
                    for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                        _hidl_vec.get(_hidl_index_0).writeEmbeddedToBlob(childBlob, _hidl_index_0 * 64);
                    }
                    _hidl_blob.putBlob(0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */, childBlob);
                }

                parcel.writeBuffer(_hidl_blob);
            }

            public final void writeEmbeddedToBlob(
                    android.os.HwBlob _hidl_blob, long _hidl_offset) {
                _hidl_blob.putInt32(_hidl_offset + 0, type);
                _hidl_blob.putInt32(_hidl_offset + 4, value_a);
                _hidl_blob.putInt32(_hidl_offset + 8, value_b);
                _hidl_blob.putBool(_hidl_offset + 12, has_buffer);
                _hidl_blob.putInt64(_hidl_offset + 16, reference);
                _hidl_blob.putInt64(_hidl_offset + 24, buffer);
                _hidl_blob.putInt64(_hidl_offset + 32, size);
                _hidl_blob.putInt32(_hidl_offset + 40, flags);
                _hidl_blob.putInt64(_hidl_offset + 48, window_offset);
                _hidl_blob.putInt64(_hidl_offset + 56, window_size);
            }
        };

        public long reference;
        public int started;
        public final vendor.trustonic.tee.V1_0.ITee.Operation.Param[/* 4 */] params = new vendor.trustonic.tee.V1_0.ITee.Operation.Param[4];

        @Override
        public final boolean equals(Object otherObject) {
            if (this == otherObject) {
                return true;
            }
            if (otherObject == null) {
                return false;
            }
            if (otherObject.getClass() != vendor.trustonic.tee.V1_0.ITee.Operation.class) {
                return false;
            }
            vendor.trustonic.tee.V1_0.ITee.Operation other = (vendor.trustonic.tee.V1_0.ITee.Operation)otherObject;
            if (this.reference != other.reference) {
                return false;
            }
            if (this.started != other.started) {
                return false;
            }
            if (!android.os.HidlSupport.deepEquals(this.params, other.params)) {
                return false;
            }
            return true;
        }

        @Override
        public final int hashCode() {
            return java.util.Objects.hash(
                    android.os.HidlSupport.deepHashCode(this.reference),
                    android.os.HidlSupport.deepHashCode(this.started),
                    android.os.HidlSupport.deepHashCode(this.params));
        }

        @Override
        public final String toString() {
            java.lang.StringBuilder builder = new java.lang.StringBuilder();
            builder.append("{");
            builder.append(".reference = ");
            builder.append(this.reference);
            builder.append(", .started = ");
            builder.append(this.started);
            builder.append(", .params = ");
            builder.append(java.util.Arrays.toString(this.params));
            builder.append("}");
            return builder.toString();
        }

        public final void readFromParcel(android.os.HwParcel parcel) {
            android.os.HwBlob blob = parcel.readBuffer(272/* size */);
            readEmbeddedFromParcel(parcel, blob, 0 /* parentOffset */);
        }

        public static final java.util.ArrayList<Operation> readVectorFromParcel(android.os.HwParcel parcel) {
            java.util.ArrayList<Operation> _hidl_vec = new java.util.ArrayList();
            android.os.HwBlob _hidl_blob = parcel.readBuffer(16 /* sizeof hidl_vec<T> */);

            {
                int _hidl_vec_size = _hidl_blob.getInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */);
                android.os.HwBlob childBlob = parcel.readEmbeddedBuffer(
                        _hidl_vec_size * 272,_hidl_blob.handle(),
                        0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */,true /* nullable */);

                _hidl_vec.clear();
                for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                    final vendor.trustonic.tee.V1_0.ITee.Operation _hidl_vec_element = new vendor.trustonic.tee.V1_0.ITee.Operation();
                    _hidl_vec_element.readEmbeddedFromParcel(parcel, childBlob, _hidl_index_0 * 272);
                    _hidl_vec.add(_hidl_vec_element);
                }
            }

            return _hidl_vec;
        }

        public final void readEmbeddedFromParcel(
                android.os.HwParcel parcel, android.os.HwBlob _hidl_blob, long _hidl_offset) {
            reference = _hidl_blob.getInt64(_hidl_offset + 0);
            started = _hidl_blob.getInt32(_hidl_offset + 8);
            {
                long _hidl_array_offset_0 = _hidl_offset + 16;
                for (int _hidl_index_0_0 = 0; _hidl_index_0_0 < 4; ++_hidl_index_0_0) {
                    params[_hidl_index_0_0] = new vendor.trustonic.tee.V1_0.ITee.Operation.Param();
                    params[_hidl_index_0_0].readEmbeddedFromParcel(parcel, _hidl_blob, _hidl_array_offset_0);
                    _hidl_array_offset_0 += 64;
                }
            }
        }

        public final void writeToParcel(android.os.HwParcel parcel) {
            android.os.HwBlob _hidl_blob = new android.os.HwBlob(272 /* size */);
            writeEmbeddedToBlob(_hidl_blob, 0 /* parentOffset */);
            parcel.writeBuffer(_hidl_blob);
        }

        public static final void writeVectorToParcel(
                android.os.HwParcel parcel, java.util.ArrayList<Operation> _hidl_vec) {
            android.os.HwBlob _hidl_blob = new android.os.HwBlob(16 /* sizeof(hidl_vec<T>) */);
            {
                int _hidl_vec_size = _hidl_vec.size();
                _hidl_blob.putInt32(0 + 8 /* offsetof(hidl_vec<T>, mSize) */, _hidl_vec_size);
                _hidl_blob.putBool(0 + 12 /* offsetof(hidl_vec<T>, mOwnsBuffer) */, false);
                android.os.HwBlob childBlob = new android.os.HwBlob((int)(_hidl_vec_size * 272));
                for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                    _hidl_vec.get(_hidl_index_0).writeEmbeddedToBlob(childBlob, _hidl_index_0 * 272);
                }
                _hidl_blob.putBlob(0 + 0 /* offsetof(hidl_vec<T>, mBuffer) */, childBlob);
            }

            parcel.writeBuffer(_hidl_blob);
        }

        public final void writeEmbeddedToBlob(
                android.os.HwBlob _hidl_blob, long _hidl_offset) {
            _hidl_blob.putInt64(_hidl_offset + 0, reference);
            _hidl_blob.putInt32(_hidl_offset + 8, started);
            {
                long _hidl_array_offset_0 = _hidl_offset + 16;
                for (int _hidl_index_0_0 = 0; _hidl_index_0_0 < 4; ++_hidl_index_0_0) {
                    params[_hidl_index_0_0].writeEmbeddedToBlob(_hidl_blob, _hidl_array_offset_0);
                    _hidl_array_offset_0 += 64;
                }
            }
        }
    };

    void flushAll()
        throws android.os.RemoteException;
    void flushPid(int client_pid)
        throws android.os.RemoteException;
    int TEEC_InitializeContext(int client_pid, long context, String name, boolean has_name)
        throws android.os.RemoteException;
    void TEEC_FinalizeContext(int client_pid, long context)
        throws android.os.RemoteException;
    int TEEC_RegisterSharedMemory(int client_pid, long context, vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem)
        throws android.os.RemoteException;
    void TEEC_ReleaseSharedMemory(int client_pid, long context, vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface TEEC_OpenSessionCallback {
        public void onValues(int return_value, int return_origin, int id, vendor.trustonic.tee.V1_0.ITee.Operation operation);
    }

    void TEEC_OpenSession(int client_pid, long context, long session, java.util.ArrayList<Byte> uuid, int login_type, java.util.ArrayList<Byte> login_data, vendor.trustonic.tee.V1_0.ITee.Operation operation, TEEC_OpenSessionCallback _hidl_cb)
        throws android.os.RemoteException;
    void TEEC_CloseSession(int client_pid, long context, long session)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface TEEC_InvokeCommandCallback {
        public void onValues(int return_value, int return_origin, vendor.trustonic.tee.V1_0.ITee.Operation operation);
    }

    void TEEC_InvokeCommand(int client_pid, long context, long session, int cmd_id, vendor.trustonic.tee.V1_0.ITee.Operation operation, TEEC_InvokeCommandCallback _hidl_cb)
        throws android.os.RemoteException;
    void TEEC_RequestCancellation(int client_pid, long context, long session, long operation)
        throws android.os.RemoteException;
    int mcOpenDevice(int client_pid)
        throws android.os.RemoteException;
    int mcCloseDevice(int client_pid)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface mcOpenSessionCallback {
        public void onValues(int mc_result, int id);
    }

    void mcOpenSession(int client_pid, java.util.ArrayList<Byte> uuid, long tci, int tci_len, mcOpenSessionCallback _hidl_cb)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface mcOpenTrustletCallback {
        public void onValues(int mc_result, int id);
    }

    void mcOpenTrustlet(int client_pid, int spid, long ta, int ta_len, long tci, int tci_len, mcOpenTrustletCallback _hidl_cb)
        throws android.os.RemoteException;
    int mcCloseSession(int client_pid, int id)
        throws android.os.RemoteException;
    int mcNotify(int client_pid, int id)
        throws android.os.RemoteException;
    int mcWaitNotification(int client_pid, int id, int timeout, boolean partial)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface mcMapCallback {
        public void onValues(int mc_result, long reference);
    }

    void mcMap(int client_pid, int id, long buf, int buf_len, mcMapCallback _hidl_cb)
        throws android.os.RemoteException;
    int mcUnmap(int client_pid, int id, long reference)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface mcGetSessionErrorCodeCallback {
        public void onValues(int mc_result, int exit_code);
    }

    void mcGetSessionErrorCode(int client_pid, int id, mcGetSessionErrorCodeCallback _hidl_cb)
        throws android.os.RemoteException;

    @java.lang.FunctionalInterface
    public interface mcGetMobiCoreVersionCallback {
        public void onValues(int mc_result, String product_id, int mci, int so, int mclf, int container, int mc_config, int tl_api, int dr_api, int cmp);
    }

    void mcGetMobiCoreVersion(int client_pid, mcGetMobiCoreVersionCallback _hidl_cb)
        throws android.os.RemoteException;
    int notifyReeEvent(int code)
        throws android.os.RemoteException;
    void registerTuiCallback(vendor.trustonic.tee.V1_0.ITuiCallback callback)
        throws android.os.RemoteException;
    java.util.ArrayList<String> interfaceChain()
        throws android.os.RemoteException;
    String interfaceDescriptor()
        throws android.os.RemoteException;
    java.util.ArrayList<byte[/* 32 */]> getHashChain()
        throws android.os.RemoteException;
    void setHALInstrumentation()
        throws android.os.RemoteException;
    boolean linkToDeath(android.os.IHwBinder.DeathRecipient recipient, long cookie)
        throws android.os.RemoteException;
    void ping()
        throws android.os.RemoteException;
    android.hidl.base.V1_0.DebugInfo getDebugInfo()
        throws android.os.RemoteException;
    void notifySyspropsChanged()
        throws android.os.RemoteException;
    boolean unlinkToDeath(android.os.IHwBinder.DeathRecipient recipient)
        throws android.os.RemoteException;

    public static final class Proxy implements ITee {
        private android.os.IHwBinder mRemote;

        public Proxy(android.os.IHwBinder remote) {
            mRemote = java.util.Objects.requireNonNull(remote);
        }

        @Override
        public android.os.IHwBinder asBinder() {
            return mRemote;
        }

        @Override
        public String toString() {
            try {
                return this.interfaceDescriptor() + "@Proxy";
            } catch (android.os.RemoteException ex) {
                /* ignored; handled below. */
            }
            return "[class or subclass of " + ITee.kInterfaceName + "]@Proxy";
        }

        @Override
        public final boolean equals(java.lang.Object other) {
            return android.os.HidlSupport.interfacesEqual(this, other);
        }

        @Override
        public final int hashCode() {
            return this.asBinder().hashCode();
        }

        // Methods from ::vendor::trustonic::tee::V1_0::ITee follow.
        @Override
        public void flushAll()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(1 /* flushAll */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void flushPid(int client_pid)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(2 /* flushPid */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int TEEC_InitializeContext(int client_pid, long context, String name, boolean has_name)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            _hidl_request.writeString(name);
            _hidl_request.writeBool(has_name);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(3 /* TEEC_InitializeContext */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_return_value = _hidl_reply.readInt32();
                return _hidl_out_return_value;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_FinalizeContext(int client_pid, long context)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(4 /* TEEC_FinalizeContext */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int TEEC_RegisterSharedMemory(int client_pid, long context, vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            shr_mem.writeToParcel(_hidl_request);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(5 /* TEEC_RegisterSharedMemory */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_return_value = _hidl_reply.readInt32();
                return _hidl_out_return_value;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_ReleaseSharedMemory(int client_pid, long context, vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            shr_mem.writeToParcel(_hidl_request);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(6 /* TEEC_ReleaseSharedMemory */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_OpenSession(int client_pid, long context, long session, java.util.ArrayList<Byte> uuid, int login_type, java.util.ArrayList<Byte> login_data, vendor.trustonic.tee.V1_0.ITee.Operation operation, TEEC_OpenSessionCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            _hidl_request.writeInt64(session);
            _hidl_request.writeInt8Vector(uuid);
            _hidl_request.writeInt32(login_type);
            _hidl_request.writeInt8Vector(login_data);
            operation.writeToParcel(_hidl_request);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(7 /* TEEC_OpenSession */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_return_value = _hidl_reply.readInt32();
                int _hidl_out_return_origin = _hidl_reply.readInt32();
                int _hidl_out_id = _hidl_reply.readInt32();
                vendor.trustonic.tee.V1_0.ITee.Operation _hidl_out_operation = new vendor.trustonic.tee.V1_0.ITee.Operation();
                _hidl_out_operation.readFromParcel(_hidl_reply);
                _hidl_cb.onValues(_hidl_out_return_value, _hidl_out_return_origin, _hidl_out_id, _hidl_out_operation);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_CloseSession(int client_pid, long context, long session)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            _hidl_request.writeInt64(session);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(8 /* TEEC_CloseSession */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_InvokeCommand(int client_pid, long context, long session, int cmd_id, vendor.trustonic.tee.V1_0.ITee.Operation operation, TEEC_InvokeCommandCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            _hidl_request.writeInt64(session);
            _hidl_request.writeInt32(cmd_id);
            operation.writeToParcel(_hidl_request);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(9 /* TEEC_InvokeCommand */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_return_value = _hidl_reply.readInt32();
                int _hidl_out_return_origin = _hidl_reply.readInt32();
                vendor.trustonic.tee.V1_0.ITee.Operation _hidl_out_operation = new vendor.trustonic.tee.V1_0.ITee.Operation();
                _hidl_out_operation.readFromParcel(_hidl_reply);
                _hidl_cb.onValues(_hidl_out_return_value, _hidl_out_return_origin, _hidl_out_operation);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void TEEC_RequestCancellation(int client_pid, long context, long session, long operation)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt64(context);
            _hidl_request.writeInt64(session);
            _hidl_request.writeInt64(operation);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(10 /* TEEC_RequestCancellation */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcOpenDevice(int client_pid)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(11 /* mcOpenDevice */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcCloseDevice(int client_pid)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(12 /* mcCloseDevice */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void mcOpenSession(int client_pid, java.util.ArrayList<Byte> uuid, long tci, int tci_len, mcOpenSessionCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt8Vector(uuid);
            _hidl_request.writeInt64(tci);
            _hidl_request.writeInt32(tci_len);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(13 /* mcOpenSession */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                int _hidl_out_id = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_mc_result, _hidl_out_id);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void mcOpenTrustlet(int client_pid, int spid, long ta, int ta_len, long tci, int tci_len, mcOpenTrustletCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(spid);
            _hidl_request.writeInt64(ta);
            _hidl_request.writeInt32(ta_len);
            _hidl_request.writeInt64(tci);
            _hidl_request.writeInt32(tci_len);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(14 /* mcOpenTrustlet */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                int _hidl_out_id = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_mc_result, _hidl_out_id);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcCloseSession(int client_pid, int id)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(15 /* mcCloseSession */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcNotify(int client_pid, int id)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(16 /* mcNotify */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcWaitNotification(int client_pid, int id, int timeout, boolean partial)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);
            _hidl_request.writeInt32(timeout);
            _hidl_request.writeBool(partial);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(17 /* mcWaitNotification */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void mcMap(int client_pid, int id, long buf, int buf_len, mcMapCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);
            _hidl_request.writeInt64(buf);
            _hidl_request.writeInt32(buf_len);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(18 /* mcMap */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                long _hidl_out_reference = _hidl_reply.readInt64();
                _hidl_cb.onValues(_hidl_out_mc_result, _hidl_out_reference);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int mcUnmap(int client_pid, int id, long reference)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);
            _hidl_request.writeInt64(reference);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(19 /* mcUnmap */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                return _hidl_out_mc_result;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void mcGetSessionErrorCode(int client_pid, int id, mcGetSessionErrorCodeCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);
            _hidl_request.writeInt32(id);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(20 /* mcGetSessionErrorCode */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                int _hidl_out_exit_code = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_mc_result, _hidl_out_exit_code);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void mcGetMobiCoreVersion(int client_pid, mcGetMobiCoreVersionCallback _hidl_cb)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(client_pid);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(21 /* mcGetMobiCoreVersion */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_mc_result = _hidl_reply.readInt32();
                String _hidl_out_product_id = _hidl_reply.readString();
                int _hidl_out_mci = _hidl_reply.readInt32();
                int _hidl_out_so = _hidl_reply.readInt32();
                int _hidl_out_mclf = _hidl_reply.readInt32();
                int _hidl_out_container = _hidl_reply.readInt32();
                int _hidl_out_mc_config = _hidl_reply.readInt32();
                int _hidl_out_tl_api = _hidl_reply.readInt32();
                int _hidl_out_dr_api = _hidl_reply.readInt32();
                int _hidl_out_cmp = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_mc_result, _hidl_out_product_id, _hidl_out_mci, _hidl_out_so, _hidl_out_mclf, _hidl_out_container, _hidl_out_mc_config, _hidl_out_tl_api, _hidl_out_dr_api, _hidl_out_cmp);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int notifyReeEvent(int code)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeInt32(code);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(22 /* notifyReeEvent */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_return_value = _hidl_reply.readInt32();
                return _hidl_out_return_value;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void registerTuiCallback(vendor.trustonic.tee.V1_0.ITuiCallback callback)
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);
            _hidl_request.writeStrongBinder(callback == null ? null : callback.asBinder());

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(23 /* registerTuiCallback */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        // Methods from ::android::hidl::base::V1_0::IBase follow.
        @Override
        public java.util.ArrayList<String> interfaceChain()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(256067662 /* interfaceChain */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                java.util.ArrayList<String> _hidl_out_descriptors = _hidl_reply.readStringVector();
                return _hidl_out_descriptors;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public String interfaceDescriptor()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(256136003 /* interfaceDescriptor */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                String _hidl_out_descriptor = _hidl_reply.readString();
                return _hidl_out_descriptor;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public java.util.ArrayList<byte[/* 32 */]> getHashChain()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(256398152 /* getHashChain */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                java.util.ArrayList<byte[/* 32 */]> _hidl_out_hashchain =  new java.util.ArrayList<byte[/* 32 */]>();
                {
                    android.os.HwBlob _hidl_blob = _hidl_reply.readBuffer(16 /* size */);
                    {
                        int _hidl_vec_size = _hidl_blob.getInt32(0 /* offset */ + 8 /* offsetof(hidl_vec<T>, mSize) */);
                        android.os.HwBlob childBlob = _hidl_reply.readEmbeddedBuffer(
                                _hidl_vec_size * 32,_hidl_blob.handle(),
                                0 /* offset */ + 0 /* offsetof(hidl_vec<T>, mBuffer) */,true /* nullable */);

                        _hidl_out_hashchain.clear();
                        for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                            final byte[/* 32 */] _hidl_vec_element = new byte[32];
                            {
                                long _hidl_array_offset_1 = _hidl_index_0 * 32;
                                childBlob.copyToInt8Array(_hidl_array_offset_1, _hidl_vec_element, 32 /* size */);
                                _hidl_array_offset_1 += 32 * 1;
                            }
                            _hidl_out_hashchain.add(_hidl_vec_element);
                        }
                    }
                }
                return _hidl_out_hashchain;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void setHALInstrumentation()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(256462420 /* setHALInstrumentation */, _hidl_request, _hidl_reply, 1 /* oneway */);
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public boolean linkToDeath(android.os.IHwBinder.DeathRecipient recipient, long cookie)
                throws android.os.RemoteException {
            return mRemote.linkToDeath(recipient, cookie);
        }
        @Override
        public void ping()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(256921159 /* ping */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public android.hidl.base.V1_0.DebugInfo getDebugInfo()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(257049926 /* getDebugInfo */, _hidl_request, _hidl_reply, 0 /* flags */);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                android.hidl.base.V1_0.DebugInfo _hidl_out_info = new android.hidl.base.V1_0.DebugInfo();
                _hidl_out_info.readFromParcel(_hidl_reply);
                return _hidl_out_info;
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void notifySyspropsChanged()
                throws android.os.RemoteException {
            android.os.HwParcel _hidl_request = new android.os.HwParcel();
            _hidl_request.writeInterfaceToken(android.hidl.base.V1_0.IBase.kInterfaceName);

            android.os.HwParcel _hidl_reply = new android.os.HwParcel();
            try {
                mRemote.transact(257120595 /* notifySyspropsChanged */, _hidl_request, _hidl_reply, 1 /* oneway */);
                _hidl_request.releaseTemporaryStorage();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public boolean unlinkToDeath(android.os.IHwBinder.DeathRecipient recipient)
                throws android.os.RemoteException {
            return mRemote.unlinkToDeath(recipient);
        }
    }

    public static abstract class Stub extends android.os.HwBinder implements ITee {
        @Override
        public android.os.IHwBinder asBinder() {
            return this;
        }

        @Override
        public final java.util.ArrayList<String> interfaceChain() {
            return new java.util.ArrayList<String>(java.util.Arrays.asList(
                    vendor.trustonic.tee.V1_0.ITee.kInterfaceName,
                    android.hidl.base.V1_0.IBase.kInterfaceName));
        }

        @Override
        public final String interfaceDescriptor() {
            return vendor.trustonic.tee.V1_0.ITee.kInterfaceName;

        }

        @Override
        public final java.util.ArrayList<byte[/* 32 */]> getHashChain() {
            return new java.util.ArrayList<byte[/* 32 */]>(java.util.Arrays.asList(
                    new byte[/* 32 */]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} /* 0000000000000000000000000000000000000000000000000000000000000000 */,
                    new byte[/* 32 */]{-67,-38,-74,24,77,122,52,109,-90,-96,125,-64,-126,-116,-15,-102,105,111,76,-86,54,17,-59,31,46,20,86,90,20,-76,15,-39} /* bddab6184d7a346da6a07dc0828cf19a696f4caa3611c51f2e14565a14b40fd9 */));

        }

        @Override
        public final void setHALInstrumentation() {

        }

        @Override
        public final boolean linkToDeath(android.os.IHwBinder.DeathRecipient recipient, long cookie) {
            return true;
        }

        @Override
        public final void ping() {
            return;

        }

        @Override
        public final android.hidl.base.V1_0.DebugInfo getDebugInfo() {
            android.hidl.base.V1_0.DebugInfo info = new android.hidl.base.V1_0.DebugInfo();
            info.pid = android.os.HidlSupport.getPidIfSharable();
            info.ptr = 0;
            info.arch = android.hidl.base.V1_0.DebugInfo.Architecture.UNKNOWN;
            return info;
        }

        @Override
        public final void notifySyspropsChanged() {
            android.os.HwBinder.reportSyspropChanged();
        }

        @Override
        public final boolean unlinkToDeath(android.os.IHwBinder.DeathRecipient recipient) {
            return true;

        }

        @Override
        public android.os.IHwInterface queryLocalInterface(String descriptor) {
            if (kInterfaceName.equals(descriptor)) {
                return this;
            }
            return null;
        }

        public void registerAsService(String serviceName) throws android.os.RemoteException {
            registerService(serviceName);
        }

        @Override
        public String toString() {
            return this.interfaceDescriptor() + "@Stub";
        }

        @Override
        public void onTransact(int _hidl_code, android.os.HwParcel _hidl_request, final android.os.HwParcel _hidl_reply, int _hidl_flags)
                throws android.os.RemoteException {
            switch (_hidl_code) {
                case 1 /* flushAll */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    flushAll();
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 2 /* flushPid */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    flushPid(client_pid);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 3 /* TEEC_InitializeContext */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    String name = _hidl_request.readString();
                    boolean has_name = _hidl_request.readBool();
                    int _hidl_out_return_value = TEEC_InitializeContext(client_pid, context, name, has_name);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_return_value);
                    _hidl_reply.send();
                    break;
                }

                case 4 /* TEEC_FinalizeContext */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    TEEC_FinalizeContext(client_pid, context);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 5 /* TEEC_RegisterSharedMemory */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem = new vendor.trustonic.tee.V1_0.ITee.GpSharedMemory();
                    shr_mem.readFromParcel(_hidl_request);
                    int _hidl_out_return_value = TEEC_RegisterSharedMemory(client_pid, context, shr_mem);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_return_value);
                    _hidl_reply.send();
                    break;
                }

                case 6 /* TEEC_ReleaseSharedMemory */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    vendor.trustonic.tee.V1_0.ITee.GpSharedMemory shr_mem = new vendor.trustonic.tee.V1_0.ITee.GpSharedMemory();
                    shr_mem.readFromParcel(_hidl_request);
                    TEEC_ReleaseSharedMemory(client_pid, context, shr_mem);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 7 /* TEEC_OpenSession */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    long session = _hidl_request.readInt64();
                    java.util.ArrayList<Byte> uuid = _hidl_request.readInt8Vector();
                    int login_type = _hidl_request.readInt32();
                    java.util.ArrayList<Byte> login_data = _hidl_request.readInt8Vector();
                    vendor.trustonic.tee.V1_0.ITee.Operation operation = new vendor.trustonic.tee.V1_0.ITee.Operation();
                    operation.readFromParcel(_hidl_request);
                    TEEC_OpenSession(client_pid, context, session, uuid, login_type, login_data, operation, new TEEC_OpenSessionCallback() {
                        @Override
                        public void onValues(int return_value, int return_origin, int id, vendor.trustonic.tee.V1_0.ITee.Operation operation) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(return_value);
                            _hidl_reply.writeInt32(return_origin);
                            _hidl_reply.writeInt32(id);
                            operation.writeToParcel(_hidl_reply);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 8 /* TEEC_CloseSession */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    long session = _hidl_request.readInt64();
                    TEEC_CloseSession(client_pid, context, session);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 9 /* TEEC_InvokeCommand */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    long session = _hidl_request.readInt64();
                    int cmd_id = _hidl_request.readInt32();
                    vendor.trustonic.tee.V1_0.ITee.Operation operation = new vendor.trustonic.tee.V1_0.ITee.Operation();
                    operation.readFromParcel(_hidl_request);
                    TEEC_InvokeCommand(client_pid, context, session, cmd_id, operation, new TEEC_InvokeCommandCallback() {
                        @Override
                        public void onValues(int return_value, int return_origin, vendor.trustonic.tee.V1_0.ITee.Operation operation) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(return_value);
                            _hidl_reply.writeInt32(return_origin);
                            operation.writeToParcel(_hidl_reply);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 10 /* TEEC_RequestCancellation */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    long context = _hidl_request.readInt64();
                    long session = _hidl_request.readInt64();
                    long operation = _hidl_request.readInt64();
                    TEEC_RequestCancellation(client_pid, context, session, operation);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 11 /* mcOpenDevice */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int _hidl_out_mc_result = mcOpenDevice(client_pid);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 12 /* mcCloseDevice */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int _hidl_out_mc_result = mcCloseDevice(client_pid);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 13 /* mcOpenSession */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    java.util.ArrayList<Byte> uuid = _hidl_request.readInt8Vector();
                    long tci = _hidl_request.readInt64();
                    int tci_len = _hidl_request.readInt32();
                    mcOpenSession(client_pid, uuid, tci, tci_len, new mcOpenSessionCallback() {
                        @Override
                        public void onValues(int mc_result, int id) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(mc_result);
                            _hidl_reply.writeInt32(id);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 14 /* mcOpenTrustlet */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int spid = _hidl_request.readInt32();
                    long ta = _hidl_request.readInt64();
                    int ta_len = _hidl_request.readInt32();
                    long tci = _hidl_request.readInt64();
                    int tci_len = _hidl_request.readInt32();
                    mcOpenTrustlet(client_pid, spid, ta, ta_len, tci, tci_len, new mcOpenTrustletCallback() {
                        @Override
                        public void onValues(int mc_result, int id) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(mc_result);
                            _hidl_reply.writeInt32(id);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 15 /* mcCloseSession */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    int _hidl_out_mc_result = mcCloseSession(client_pid, id);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 16 /* mcNotify */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    int _hidl_out_mc_result = mcNotify(client_pid, id);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 17 /* mcWaitNotification */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    int timeout = _hidl_request.readInt32();
                    boolean partial = _hidl_request.readBool();
                    int _hidl_out_mc_result = mcWaitNotification(client_pid, id, timeout, partial);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 18 /* mcMap */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    long buf = _hidl_request.readInt64();
                    int buf_len = _hidl_request.readInt32();
                    mcMap(client_pid, id, buf, buf_len, new mcMapCallback() {
                        @Override
                        public void onValues(int mc_result, long reference) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(mc_result);
                            _hidl_reply.writeInt64(reference);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 19 /* mcUnmap */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    long reference = _hidl_request.readInt64();
                    int _hidl_out_mc_result = mcUnmap(client_pid, id, reference);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_mc_result);
                    _hidl_reply.send();
                    break;
                }

                case 20 /* mcGetSessionErrorCode */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    int id = _hidl_request.readInt32();
                    mcGetSessionErrorCode(client_pid, id, new mcGetSessionErrorCodeCallback() {
                        @Override
                        public void onValues(int mc_result, int exit_code) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(mc_result);
                            _hidl_reply.writeInt32(exit_code);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 21 /* mcGetMobiCoreVersion */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int client_pid = _hidl_request.readInt32();
                    mcGetMobiCoreVersion(client_pid, new mcGetMobiCoreVersionCallback() {
                        @Override
                        public void onValues(int mc_result, String product_id, int mci, int so, int mclf, int container, int mc_config, int tl_api, int dr_api, int cmp) {
                            _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                            _hidl_reply.writeInt32(mc_result);
                            _hidl_reply.writeString(product_id);
                            _hidl_reply.writeInt32(mci);
                            _hidl_reply.writeInt32(so);
                            _hidl_reply.writeInt32(mclf);
                            _hidl_reply.writeInt32(container);
                            _hidl_reply.writeInt32(mc_config);
                            _hidl_reply.writeInt32(tl_api);
                            _hidl_reply.writeInt32(dr_api);
                            _hidl_reply.writeInt32(cmp);
                            _hidl_reply.send();
                            }});
                    break;
                }

                case 22 /* notifyReeEvent */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    int code = _hidl_request.readInt32();
                    int _hidl_out_return_value = notifyReeEvent(code);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeInt32(_hidl_out_return_value);
                    _hidl_reply.send();
                    break;
                }

                case 23 /* registerTuiCallback */:
                {
                    _hidl_request.enforceInterface(vendor.trustonic.tee.V1_0.ITee.kInterfaceName);

                    vendor.trustonic.tee.V1_0.ITuiCallback callback = vendor.trustonic.tee.V1_0.ITuiCallback.asInterface(_hidl_request.readStrongBinder());
                    registerTuiCallback(callback);
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 256067662 /* interfaceChain */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    java.util.ArrayList<String> _hidl_out_descriptors = interfaceChain();
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeStringVector(_hidl_out_descriptors);
                    _hidl_reply.send();
                    break;
                }

                case 256131655 /* debug */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.send();
                    break;
                }

                case 256136003 /* interfaceDescriptor */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    String _hidl_out_descriptor = interfaceDescriptor();
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_reply.writeString(_hidl_out_descriptor);
                    _hidl_reply.send();
                    break;
                }

                case 256398152 /* getHashChain */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    java.util.ArrayList<byte[/* 32 */]> _hidl_out_hashchain = getHashChain();
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    {
                        android.os.HwBlob _hidl_blob = new android.os.HwBlob(16 /* size */);
                        {
                            int _hidl_vec_size = _hidl_out_hashchain.size();
                            _hidl_blob.putInt32(0 /* offset */ + 8 /* offsetof(hidl_vec<T>, mSize) */, _hidl_vec_size);
                            _hidl_blob.putBool(0 /* offset */ + 12 /* offsetof(hidl_vec<T>, mOwnsBuffer) */, false);
                            android.os.HwBlob childBlob = new android.os.HwBlob((int)(_hidl_vec_size * 32));
                            for (int _hidl_index_0 = 0; _hidl_index_0 < _hidl_vec_size; ++_hidl_index_0) {
                                {
                                    long _hidl_array_offset_1 = _hidl_index_0 * 32;
                                    childBlob.putInt8Array(_hidl_array_offset_1, _hidl_out_hashchain.get(_hidl_index_0));
                                    _hidl_array_offset_1 += 32 * 1;
                                }
                            }
                            _hidl_blob.putBlob(0 /* offset */ + 0 /* offsetof(hidl_vec<T>, mBuffer) */, childBlob);
                        }
                        _hidl_reply.writeBuffer(_hidl_blob);
                    }
                    _hidl_reply.send();
                    break;
                }

                case 256462420 /* setHALInstrumentation */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    setHALInstrumentation();
                    break;
                }

                case 256660548 /* linkToDeath */:
                {
                break;
                }

                case 256921159 /* ping */:
                {
                break;
                }

                case 257049926 /* getDebugInfo */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    android.hidl.base.V1_0.DebugInfo _hidl_out_info = getDebugInfo();
                    _hidl_reply.writeStatus(android.os.HwParcel.STATUS_SUCCESS);
                    _hidl_out_info.writeToParcel(_hidl_reply);
                    _hidl_reply.send();
                    break;
                }

                case 257120595 /* notifySyspropsChanged */:
                {
                    _hidl_request.enforceInterface(android.hidl.base.V1_0.IBase.kInterfaceName);

                    notifySyspropsChanged();
                    break;
                }

                case 257250372 /* unlinkToDeath */:
                {
                break;
                }

            }
        }
    }
}
