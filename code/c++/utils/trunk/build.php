<?php
#config 
$dst_dir="/search/dev/meta/mc/trunk2";
$module_name="metacache";
$version="http://svn.sogou-inc.com/svn/websearch4/web/web_metacache/trunk";
#mk dir for src code
system("mkdir ".$dst_dir."/".$module);

#get dailybuild file
system("mkdir ".$dst_dir."/".$module."/dailybuild/");
system("svn co http://svn.sogou-inc.com/svn/dailybuild/ct-test ".$dst_dir."/".$module."/dailybuild/ --username svnreader --password sogouorz");
$dailybuild_dir=$dst_dir."/".$module."/dailybuild";

# install the require packages
$module=$module_name.".ini";
$dailybuild_file=$dailybuild_dir."/".$module;
$requires=system("grep build_requires ".$dailybuild_file."|awk -F = '{print $2}'|awk -F ' ' '{print $0}'");
echo $requires;
$requires_array=explode(" ",$requires);
//print_r($requires_array);
exec("yum list installed",$installed);
$arch=system("uname -m");
//print_r($installed);
foreach ($requires_array as $require) {
	$need_install=1;
	foreach ($installed as $install) {
		$pos=strpos($install,$require.".".$arch,0);
		//$pos=strpos($install,"zip.x86_64ddddd",0);
		if ($pos === false ){
		}else{ 
			echo $require.".".$arch." had been installed\n";
			$need_install=0;
			break;
		}

	} 
	if ($need_install == 1){
			echo "to install ".$require."\n";
			system("yum install ".$require.".".$arch." -y");
	}
}
#build the modules
exec("cat ".$dailybuild_file,$file_content);
$find_svn_tag=0;
foreach ($file_content as $fline) {

	if ($fline[0]==";"){
		continue;
	}
	$pos=strpos($fline,"[main.svn]");
	if ($pos === false){
	}else{
		$find_svn_tag=1;
		continue;
	}
	if ($find_svn_tag!=1)
		continue;
	
	//echo "#################".$fline."\n";
	$svn_array=explode("=",$fline);	
	//print_r($svn_array);
	if( $svn_array[0][0] == ".")
	{
		if (!empty($version))
		{
			echo "#################".$version."\n";
			//system("svn co ".$version." ".$dst_dir."/".$module_name."  --username svnreader --password sogouorz");
			system("svn co ".$version." ".$dst_dir."  --username svnreader --password sogouorz");
		}else 
		{
			echo "#################        no ".$version."\n";
			system("svn co ".$svn_array[1]." ".$dst_dir."  --username svnreader --password sogouorz");
		}
	}
	else if ($svn_array[0])
	{
		system("svn co ".$svn_array[1]." ".$dst_dir."/".$svn_array[0]."  --username svnreader --password sogouorz");
	}
	
}
system("cd ".$dst_dir.";make");

?>
