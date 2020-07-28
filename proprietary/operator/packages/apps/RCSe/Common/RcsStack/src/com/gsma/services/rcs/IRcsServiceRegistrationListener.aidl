package com.gsma.services.rcs;


/**
 * RCS service registration events listener
 */
interface IRcsServiceRegistrationListener {

	void onServiceRegistered();

	void onServiceUnregistered(in int reasonCode);
}