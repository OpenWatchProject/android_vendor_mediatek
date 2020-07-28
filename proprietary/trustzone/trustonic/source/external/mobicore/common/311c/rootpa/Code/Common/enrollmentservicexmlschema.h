/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
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

#ifndef ENROLLMENTSERVICEXMLSCHEMA_H
#define ENROLLMENTSERVICEXMLSCHEMA_H

#define ENROLLMENT_SERVICE_XSD_NAME  "EnrollmentService.xsd"
#define PLATFORM_TYPES_XSD_NAME      "MCPlatformTypes.xsd"

#define ENROLLMENT_SERVICE_XSD "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<xsd:schema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" \
	targetNamespace=\"http://www.mcore.gi-de.com/2012/04/schema/EnrollmentService\" \
	xmlns:mces=\"http://www.mcore.gi-de.com/2012/04/schema/EnrollmentService\" \
	xmlns:mcpt=\"http://www.mcore.gi-de.com/2012/02/schema/MCPlatformTypes\" \
	elementFormDefault=\"qualified\"> \
 \
	<xsd:import namespace=\"http://www.mcore.gi-de.com/2012/02/schema/MCPlatformTypes\" \
		schemaLocation=\"MCPlatformTypes.xsd\" /> \
 \
	<xsd:simpleType name=\"CommandType\"> \
		<xsd:restriction base=\"xsd:string\"> \
			<xsd:enumeration value=\"CMP\" /> \
			<xsd:enumeration value=\"SO_UPLOAD\" /> \
			<xsd:enumeration value=\"TLT_UPLOAD\" /> \
		</xsd:restriction> \
	</xsd:simpleType> \
 \
	<xsd:complexType name=\"Command\"> \
		<xsd:sequence> \
			<xsd:element name=\"commandValue\" type=\"xsd:base64Binary\"/> \
		</xsd:sequence> \
		<xsd:attribute name=\"id\" type=\"xsd:int\" use=\"required\" /> \
		<xsd:attribute name=\"type\" type=\"mces:CommandType\" use=\"required\" /> \
		<xsd:attribute name=\"ignoreError\" type=\"xsd:boolean\" \
			default=\"false\" /> \
	</xsd:complexType> \
 \
	<xsd:simpleType name=\"CommandExecutionErrorCode\"> \
		<xsd:restriction base=\"xsd:string\"> \
			<xsd:enumeration value=\"COMMAND_NOT_SUPPORTED_ERROR\" /> \
			<xsd:enumeration value=\"INTERNAL_ERROR\" /> \
			<xsd:enumeration value=\"BUSY_ERROR\" /> \
			<xsd:enumeration value=\"REGISTRY_ERROR\" /> \
            <xsd:enumeration value=\"REGISTRY_OBJECT_NOT_AVAILABLE\" /> \
			<xsd:enumeration value=\"MOBICORE_CONNECTION_ERROR\" /> \
			<xsd:enumeration value=\"OUT_OF_MEMORY_ERROR\" /> \
			<xsd:enumeration value=\"XML_ERROR\" /> \
        </xsd:restriction> \
	</xsd:simpleType> \
 \
	<xsd:complexType name=\"CommandExecutionError\"> \
		<xsd:attribute name=\"errorCode\" type=\"mces:CommandExecutionErrorCode\" \
			use=\"required\" /> \
		<xsd:attribute name=\"errorDetail\" type=\"xsd:int\" /> \
	</xsd:complexType> \
 \
	<xsd:complexType name=\"CommandResult\"> \
		<xsd:sequence> \
			<xsd:choice> \
				<xsd:element name=\"resultValue\" type=\"xsd:base64Binary\" /> \
				<xsd:element name=\"resultError\" type=\"mces:CommandExecutionError\" /> \
			</xsd:choice> \
		</xsd:sequence> \
		<xsd:attribute name=\"id\" type=\"xsd:int\" use=\"required\" /> \
	</xsd:complexType> \
 \
  	<xsd:complexType name=\"TrustletEncryptionKey\"> \
		<xsd:simpleContent> \
			<xsd:extension base=\"xsd:base64Binary\"> \
				<xsd:attribute name=\"minTltVersion\" type=\"mcpt:Version\" use=\"required\" /> \
				<xsd:attribute name=\"tltPukHash\" type=\"xsd:base64Binary\" use=\"required\" /> \
			</xsd:extension> \
		</xsd:simpleContent> \
	</xsd:complexType> \
\
	<xsd:simpleType name=\"TrustletMemoryType\"> \
		<xsd:restriction base=\"xsd:int\"> \
			<xsd:enumeration value=\"0\"> \
				<xsd:annotation> \
					<xsd:documentation> \
	If enough space is available in the internal \
	memory the trustlet will be loaded into the internal memory, else into the external \
					</xsd:documentation> \
				</xsd:annotation> \
			</xsd:enumeration> \
			<xsd:enumeration value=\"1\"> \
				<xsd:annotation> \
					<xsd:documentation> \
						Use internal memory only. \
					</xsd:documentation> \
				</xsd:annotation> \
			</xsd:enumeration> \
			<xsd:enumeration value=\"2\"> \
				<xsd:annotation> \
					<xsd:documentation> \
						Use external memory only. \
					</xsd:documentation> \
				</xsd:annotation> \
			</xsd:enumeration> \
		</xsd:restriction> \
	</xsd:simpleType> \
 \
	<xsd:complexType name=\"TrustletAXF\"> \
		<xsd:simpleContent> \
			<xsd:extension base=\"xsd:base64Binary\"> \
				<xsd:attribute name=\"minTltVersion\" type=\"mcpt:Version\" \
					use=\"required\" /> \
				<xsd:attribute name=\"tltPukHash\" type=\"xsd:base64Binary\" \
					use=\"required\" /> \
				<xsd:attribute name=\"memoryType\" type=\"mces:TrustletMemoryType\" \
					default=\"2\" /> \
				<xsd:attribute name=\"numberOfInstances\" type=\"xsd:int\" \
					default=\"1\" /> \
				<xsd:attribute name=\"flags\" type=\"xsd:int\" default=\"0\" /> \
			</xsd:extension> \
		</xsd:simpleContent> \
	</xsd:complexType> \
\
	<xsd:complexType name=\"TrustletInstallationRequest\"> \
		<xsd:annotation> \
			<xsd:documentation> \
				This element defines data required to be able to \
				install a trustlet over PA. The schema below supports \
				following \
				combinations: \
				1. encrypted trustlet binary key (enables the \
				installation of the tltBin using the static tlt \
				encryption schema. \
				Trustlet remains on the device. \
				Just the key gets decrypted and \
				pushed back to the \
				device using corresponding cmp commands) \
				2. \
				encrypted tltBin (enables the installation \
				of the tltBin using the \
				dynamic tlt encryption scheme. \
				Trustlet get reencrypted by the SE \
				with a new key \
				generated by the SE during the execution of the \
				workflow and pushed back to \
				the device). \
				In both cases the key for \
				either the encryption of \
				the key or the tltBin must be known to the \
				SE \
			</xsd:documentation> \
		</xsd:annotation> \
		<xsd:sequence> \
			<xsd:choice> \
				<xsd:element name=\"trustletAxf\" type=\"mces:TrustletAXF\" /> \
				<xsd:element name=\"trustletEncryptionKey\" type=\"mces:TrustletEncryptionKey\" /> \
			</xsd:choice> \
		</xsd:sequence> \
	</xsd:complexType> \
\
	<xsd:complexType name=\"CommandResultList\"> \
		<xsd:sequence> \
			<xsd:element name=\"commandResult\" type=\"mces:CommandResult\" maxOccurs=\"unbounded\" /> \
		</xsd:sequence> \
	</xsd:complexType> \
 \
	<xsd:element name=\"ContentManagementResponse\"> \
		<xsd:complexType> \
			<xsd:annotation> \
				<xsd:documentation> \
					This element is a top level element sent by PA to \
					the SE. It contains either a SystemInformation of \
					the device the PA \
					is running on, a responses for \
					previously received \
					ContentManagementRequest, or \
					data about the trustlet to be \
					installed. \
				</xsd:documentation> \
			</xsd:annotation> \
			<xsd:choice> \
				<xsd:element name=\"systemInformation\" type=\"mcpt:SystemInformation\" /> \
				<xsd:element name=\"tltInstallationRequest\" type=\"mces:TrustletInstallationRequest\" /> \
				<xsd:element name=\"commandResultList\" type=\"mces:CommandResultList\"/> \
			</xsd:choice> \
		</xsd:complexType> \
	</xsd:element> \
 \
	<xsd:complexType name=\"CommandList\"> \
		<xsd:sequence> \
			<xsd:element name=\"command\" type=\"mces:Command\" \
				minOccurs=\"1\" maxOccurs=\"unbounded\" /> \
		</xsd:sequence> \
	</xsd:complexType> \
 \
	<xsd:element name=\"ContentManagementRequest\"> \
		<xsd:complexType> \
			<xsd:annotation> \
				<xsd:documentation> \
					This element is a top level element sent from SE \
					to PA as a result of previously received HTTP message. \
				</xsd:documentation> \
			</xsd:annotation> \
			<xsd:sequence> \
				<xsd:element name=\"commands\" type=\"mces:CommandList\"/> \
			</xsd:sequence> \
		</xsd:complexType> \
	</xsd:element> \
</xsd:schema>"

#define PLATFORM_TYPES_XSD "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<xsd:schema \
	xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"  \
	xmlns:mcpt=\"http://www.mcore.gi-de.com/2012/02/schema/MCPlatformTypes\" \
	targetNamespace=\"http://www.mcore.gi-de.com/2012/02/schema/MCPlatformTypes\" \
	elementFormDefault=\"qualified\"> \
\
	<xsd:simpleType name=\"Version\"> \
		<xsd:annotation> \
			<xsd:documentation> \
				Version of an object withing MobiCore eco system \
			</xsd:documentation> \
		</xsd:annotation> \
		<xsd:restriction base=\"xsd:int\" /> \
	</xsd:simpleType> \
\
	<xsd:complexType name=\"McVersion\"> \
		<xsd:annotation> \
			<xsd:documentation> \
				This element contains version data of MobiCore \
				components \
			</xsd:documentation> \
		</xsd:annotation> \
		<xsd:attribute name=\"productId\" type=\"xsd:string\" use=\"required\" /> \
		<xsd:attribute name=\"versionMci\" type=\"mcpt:Version\" use=\"required\" /> \
		<xsd:attribute name=\"versionSo\" type=\"mcpt:Version\" use=\"required\" /> \
		<xsd:attribute name=\"versionMclf\" type=\"mcpt:Version\" use=\"required\" /> \
		<xsd:attribute name=\"versionContainer\" type=\"mcpt:Version\" \
			use=\"required\" /> \
		<xsd:attribute name=\"versionMcConfig\" type=\"mcpt:Version\" \
			use=\"required\" /> \
		<xsd:attribute name=\"versionTlApi\" type=\"mcpt:Version\" use=\"required\" /> \
		<xsd:attribute name=\"versionDrApi\" type=\"mcpt:Version\" use=\"required\" /> \
		<xsd:attribute name=\"versionCmp\" type=\"mcpt:Version\" use=\"required\" /> \
	</xsd:complexType> \
 \
	<xsd:complexType name=\"SystemInformation\"> \
		<xsd:annotation> \
			<xsd:documentation> \
				This element contains system information of a \
				device \
			</xsd:documentation> \
		</xsd:annotation> \
		<xsd:sequence> \
			<xsd:element name=\"mcVersion\" type=\"mcpt:McVersion\" /> \
		</xsd:sequence> \
		<xsd:attribute name=\"imei\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"mno\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"brand\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"manufacturer\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"hardware\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"model\" type=\"xsd:string\" use=\"optional\" /> \
        <xsd:attribute name=\"sip\" type=\"xsd:string\" use=\"optional\" /> \
		<xsd:attribute name=\"version\" type=\"xsd:string\" use=\"optional\" /> \
	</xsd:complexType> \
</xsd:schema>"


#endif //ENROLLMENTSERVICEXMLSCHEMA_H
