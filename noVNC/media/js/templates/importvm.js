function CheckImportSetting()
{
	var msg = '';

	// check import path
	var ele = document.getElementById('ImportVMFrom').import_path;
	ele.value = ele.value.replace(/(^\s*|\s*$)/g, "");
	if (ele.value.length == 0) {
		msg += gettext('Import Path must required.')+'\n'
	}
	
	// check vm name
	ele = document.getElementById('ImportVMFrom').name;
	ele.value = ele.value.replace(/(^\s*|\s*$)/g, "");
	if (ele.value.length == 0) {
		msg += gettext('VM name must required.')+'\n';
	} else {
		if (ele.value.length > 20) {
			msg += gettext('VM name length can not be longer than 20 chars.')+'\n';
		}
		var re = /^([A-Za-z0-9_\-\.]+)$/g;
		if (!re.test(ele.value)) {
			msg += gettext('Invalid characters in VM name. Please only use:')+' a-z,A-Z,0-9,_,-,.\n';
		}
	}
	
	//check mac
	var cnt = 0;
	var RegExPattern = /^([0-9a-fA-F]{1,2}[\.:-]){5}([0-9a-fA-F]{1,2})$/;
	while (document.getElementById('mac_'+cnt)) {
		ele = document.getElementById('mac_'+cnt);
		if (!ele.value.match(RegExPattern)) {
			msg += gettext('Invalid format of MAC')+' #' + (cnt+1) + '\n';
		} else if (parseInt(ele.value.split(':')[0],16)%2){
		    msg += interpolate(gettext('First number of MAC #%(cnt)s must be even(ex.0,2,4,6,...)\n'),{'cnt':(cnt+1)},true);
		}
		cnt++;
	}

	if (msg.length > 0) {
		alert(msg)
		return false;
	}

	return true;
}


