# SPDX-License-Identifier: GPL-2.0-only

VIDEO_ROOT := $(KERNEL_SRC)/$(M)
KBUILD_OPTIONS += VIDEO_ROOT=$(VIDEO_ROOT)

all: modules

modules:
	ln -sf $(VIDEO_ROOT)/driver $(VIDEO_ROOT)/msm_video/driver
	ln -sf $(VIDEO_ROOT)/driver $(VIDEO_ROOT)/video/driver
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS)
	rm $(VIDEO_ROOT)/msm_video/driver
	rm $(VIDEO_ROOT)/video/driver

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 -C $(KERNEL_SRC) M=$(M) modules_install

%:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) $@ $(KBUILD_OPTIONS)

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd Module.symvers
	rm -rf .tmp_versions
