" Vxl now requires explicit lib prefix as of feb'06
"    When including vxl into other packages, it is necessary
"    to add a prefix to the library name in order
"    to distiguish it from other system versions.
"    
"    The following subsittusions were used:
"    
:%s/target_link_libraries *(\(.* \)vcl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vcl/ge
:%s/target_link_libraries *(\(.* \)vpgl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpgl/ge
:%s/target_link_libraries *(\(.* \)testlib\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}testlib/ge
:%s/target_link_libraries *(\(.* \)vbl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vbl/ge
:%s/target_link_libraries *(\(.* \)vbl_example_templates\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vbl_example_templates/ge
:%s/target_link_libraries *(\(.* \)vbl_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vbl_io/ge
:%s/target_link_libraries *(\(.* \)vcsl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vcsl/ge
:%s/target_link_libraries *(\(.* \)vgl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgl/ge
:%s/target_link_libraries *(\(.* \)vgl_algo\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgl_algo/ge
:%s/target_link_libraries *(\(.* \)vgl_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgl_io/ge
:%s/target_link_libraries *(\(.* \)vgl_xio\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgl_xio/ge
:%s/target_link_libraries *(\(.* \)vgui\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgui/ge
:%s/target_link_libraries *(\(.* \)vgui_wx\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vgui_wx/ge
:%s/target_link_libraries *(\(.* \)vidl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vidl/ge
:%s/target_link_libraries *(\(.* \)vidl_gui\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vidl_gui/ge
:%s/target_link_libraries *(\(.* \)vil\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil/ge
:%s/target_link_libraries *(\(.* \)vil1\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil1/ge
:%s/target_link_libraries *(\(.* \)vil1_network\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil1_network/ge
:%s/target_link_libraries *(\(.* \)vil_algo\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil_algo/ge
:%s/target_link_libraries *(\(.* \)vil_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil_io/ge
:%s/target_link_libraries *(\(.* \)vil_network\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vil_network/ge
:%s/target_link_libraries *(\(.* \)vnl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vnl/ge
:%s/target_link_libraries *(\(.* \)vnl_algo\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vnl_algo/ge
:%s/target_link_libraries *(\(.* \)vnl_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vnl_io/ge
:%s/target_link_libraries *(\(.* \)vnl_xio\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vnl_xio/ge
:%s/target_link_libraries *(\(.* \)vpdl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpdl/ge
:%s/target_link_libraries *(\(.* \)vpgl_algo\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_algo/ge
:%s/target_link_libraries *(\(.* \)vpgl_file_formats\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_file_formats/ge
:%s/target_link_libraries *(\(.* \)vpgl_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_io/ge
:%s/target_link_libraries *(\(.* \)vpgl_xio\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_xio/ge
:%s/target_link_libraries *(\(.* \)vpl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vpl/ge
:%s/target_link_libraries *(\(.* \)vsl\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vsl/ge
:%s/target_link_libraries *(\(.* \)vul_io\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vul_io/ge
:%s/target_link_libraries *(\(.* \)vul_network\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vul_network/ge
:%s/target_link_libraries *(\(.* \)vul\>/target_link_libraries(\1 ${VXL_LIB_PREFIX}vul/ge

:%s/\<link_libraries *(\(.* \)vpgl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpgl/ge
:%s/\<link_libraries *(\(.* \)testlib\>/link_libraries(\1 ${VXL_LIB_PREFIX}testlib/ge
:%s/\<link_libraries *(\(.* \)vbl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vbl/ge
:%s/\<link_libraries *(\(.* \)vbl_example_templates\>/link_libraries(\1 ${VXL_LIB_PREFIX}vbl_example_templates/ge
:%s/\<link_libraries *(\(.* \)vbl_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vbl_io/ge
:%s/\<link_libraries *(\(.* \)vcsl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vcsl/ge
:%s/\<link_libraries *(\(.* \)vgl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgl/ge
:%s/\<link_libraries *(\(.* \)vgl_algo\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgl_algo/ge
:%s/\<link_libraries *(\(.* \)vgl_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgl_io/ge
:%s/\<link_libraries *(\(.* \)vgl_xio\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgl_xio/ge
:%s/\<link_libraries *(\(.* \)vgui\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgui/ge
:%s/\<link_libraries *(\(.* \)vgui_wx\>/link_libraries(\1 ${VXL_LIB_PREFIX}vgui_wx/ge
:%s/\<link_libraries *(\(.* \)vidl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vidl/ge
:%s/\<link_libraries *(\(.* \)vidl_gui\>/link_libraries(\1 ${VXL_LIB_PREFIX}vidl_gui/ge
:%s/\<link_libraries *(\(.* \)vil\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil/ge
:%s/\<link_libraries *(\(.* \)vil1\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil1/ge
:%s/\<link_libraries *(\(.* \)vil1_network\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil1_network/ge
:%s/\<link_libraries *(\(.* \)vil_algo\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil_algo/ge
:%s/\<link_libraries *(\(.* \)vil_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil_io/ge
:%s/\<link_libraries *(\(.* \)vil_network\>/link_libraries(\1 ${VXL_LIB_PREFIX}vil_network/ge
:%s/\<link_libraries *(\(.* \)vnl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vnl/ge
:%s/\<link_libraries *(\(.* \)vnl_algo\>/link_libraries(\1 ${VXL_LIB_PREFIX}vnl_algo/ge
:%s/\<link_libraries *(\(.* \)vnl_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vnl_io/ge
:%s/\<link_libraries *(\(.* \)vnl_xio\>/link_libraries(\1 ${VXL_LIB_PREFIX}vnl_xio/ge
:%s/\<link_libraries *(\(.* \)vpdl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpdl/ge
:%s/\<link_libraries *(\(.* \)vpgl_algo\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_algo/ge
:%s/\<link_libraries *(\(.* \)vpgl_file_formats\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_file_formats/ge
:%s/\<link_libraries *(\(.* \)vpgl_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_io/ge
:%s/\<link_libraries *(\(.* \)vpgl_xio\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpgl_xio/ge
:%s/\<link_libraries *(\(.* \)vpl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vpl/ge
:%s/\<link_libraries *(\(.* \)vsl\>/link_libraries(\1 ${VXL_LIB_PREFIX}vsl/ge
:%s/\<link_libraries *(\(.* \)vul_io\>/link_libraries(\1 ${VXL_LIB_PREFIX}vul_io/ge
:%s/\<link_libraries *(\(.* \)vul_network\>/link_libraries(\1 ${VXL_LIB_PREFIX}vul_network/ge
:%s/\<link_libraries *(\(.* \)vul\>/link_libraries(\1 ${VXL_LIB_PREFIX}vul/ge
