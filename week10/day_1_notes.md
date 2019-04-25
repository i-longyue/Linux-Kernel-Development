otg20_probe_probe
	pcd_init (这个像是单纯的硬件初始化工作)
		dwc_otg_pcd_init
			alloc_wrapper

	usb_add_gadget_udc
		add_udc_to_list       /* 这里相当于把udc加入了list */
	usb_add_gadget_udc_release
		gadget_add_eps


android.c 
	init
	usb_composite_probe
	usb_gadget_probe_driver

		udc_bind_to_driver
			dwc_otg_gadget_start /* 对接了dwc_otg_pcd_linux和udc-core层 */


	composite_bind
	android_bind
		dwc_otg_pcd_linux.c  dwc_otg_pcd_pullup(0)
