ifeq ($(strip $(TRUSTKERNEL_TEE_SUPPORT)), yes)

	PRODUCT_PACKAGES += libteec
	PRODUCT_PACKAGES += teed

	# for Production Line
	PRODUCT_PACKAGES += libpl
	PRODUCT_PACKAGES += pld

	PRODUCT_PACKAGES += libkphproxy
	PRODUCT_PACKAGES += kph

	PRODUCT_PACKAGES += kmsetkey.trustkernel

	# for keybox
	PRODUCT_PACKAGES += 6B6579626F785F6372797074

	# Android build-in services
	PRODUCT_PACKAGES += gatekeeper.trustkernel

ifeq ($(strip $(TRUSTKERNEL_TEE_KEYMASTER3)), yes)
	PRODUCT_PACKAGES += keystore.v2.trustkernel
endif

	# TAs
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/02662e8e-e126-11e5-b86d9a79f06e9478.ta:vendor/app/t6/02662e8e-e126-11e5-b86d9a79f06e9478.ta
ifeq ($(strip $(TRUSTKERNEL_TEE_KEYMASTER3)), yes)
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/9ef77781-7bd5-4e39-965f20f6f211f46b.ta:vendor/app/t6/9ef77781-7bd5-4e39-965f20f6f211f46b.ta
else
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/9ef77781-7bd5-4e39-965f20f6f211f46b.km4.ta:vendor/app/t6/9ef77781-7bd5-4e39-965f20f6f211f46b.ta
endif
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/8b1e0e41-2636-11e1-ad9e0002a5d5c51b.ta:vendor/app/t6/8b1e0e41-2636-11e1-ad9e0002a5d5c51b.ta
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/b46325e6-5c90-8252-2eada8e32e5180d6.ta:vendor/app/t6/b46325e6-5c90-8252-2eada8e32e5180d6.ta
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/ta/0799a943-84a2-dead-0e3f8c88ad72507f.ta:vendor/app/t6/0799a943-84a2-dead-0e3f8c88ad72507f.ta

ifeq ($(strip $(TRUSTKERNEL_TRUSTSTORE_PROTECT)), yes)
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/bsp/platform/common/scripts/trustkernel.protect.rc:vendor/etc/init/trustkernel.rc
endif

	# for Android P device id attestation: CTS related
	PRODUCT_COPY_FILES += frameworks/native/data/etc/android.software.device_id_attestation.xml:vendor/etc/permissions/android.software.device_id_attestation.xml

	# kph configs
	PRODUCT_COPY_FILES += vendor/mediatek/proprietary/trustzone/trustkernel/source/bsp/platform/common/scripts/kph_cfg/cfg.ini:vendor/app/t6/cfg.ini
endif
