ifeq ($(call is-board-platform-in-list,pineapple sun), true)
PRODUCT_PACKAGES += qbt_handler.ko
endif
