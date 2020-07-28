/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: X:\\casws_mtk80037\\alps-mp-o1_mp1-V1_87\\vendor\\mediatek\\proprietary\\packages\\apps\\Mms\\src\\com\\mediatek\\mms\\appserviceproxy\\IAppServiceProxy.aidl
 */
package com.mediatek.mms.appserviceproxy;
public interface IAppServiceProxy extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements com.mediatek.mms.appserviceproxy.IAppServiceProxy
{
private static final java.lang.String DESCRIPTOR = "com.mediatek.mms.appserviceproxy.IAppServiceProxy";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an com.mediatek.mms.appserviceproxy.IAppServiceProxy interface,
 * generating a proxy if needed.
 */
public static com.mediatek.mms.appserviceproxy.IAppServiceProxy asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof com.mediatek.mms.appserviceproxy.IAppServiceProxy))) {
return ((com.mediatek.mms.appserviceproxy.IAppServiceProxy)iin);
}
return new com.mediatek.mms.appserviceproxy.IAppServiceProxy.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_startServiceByParam:
{
data.enforceInterface(DESCRIPTOR);
com.mediatek.mms.appserviceproxy.AppServiceProxyParam _arg0;
if ((0!=data.readInt())) {
_arg0 = com.mediatek.mms.appserviceproxy.AppServiceProxyParam.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
this.startServiceByParam(_arg0);
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements com.mediatek.mms.appserviceproxy.IAppServiceProxy
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
@Override public void startServiceByParam(com.mediatek.mms.appserviceproxy.AppServiceProxyParam param) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((param!=null)) {
_data.writeInt(1);
param.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
mRemote.transact(Stub.TRANSACTION_startServiceByParam, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_startServiceByParam = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
}
public void startServiceByParam(com.mediatek.mms.appserviceproxy.AppServiceProxyParam param) throws android.os.RemoteException;
}
