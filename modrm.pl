
#!/usr/bin/perl

# Intel's Tool Interface Standards (TIS) site
#        http://developer.intel.com/vtune/tis.htm
#        http://www.intel.com/design/pentiumii/manuals/

my %target_info;
$target_info{endian} = little;


sub decode_modrm {
	local($buf) = shift;
	local($modrm, $sib, $disp);
	local($msd_str, $md_str);

	if ( $target_info{endian} eq "big" ) {
		$msd_str = "CCN";
		$md_str = "CCN";
	} else {
		$msd_str = "CCV";
		$md_str = "CCV";
	}
	($modrm, $sib, $disp) = unpack($msd_str, $buf);
}
