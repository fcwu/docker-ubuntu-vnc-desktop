
function _callbk_update_snapshot_progress_multiLanguage(){
	return interpolate(gettext('Create snapshot [ %(snapshot_name_value)s ] successful'),{'snapshot_name_value':document.getElementById('snapshot_name_value').value},true);
}


function CheckAdvSumit()
{
    var msg = '';
    
    //check vnc port & password
    var ele = document.getElementById("adv_vnc_port");
    if (ele.options[ele.selectedIndex].value == "custom") {
        var port = document.getElementById("adv_vnc_port_text").value;
        var RegExPattern = /^[0-9]+$/;
        if (!port.match(RegExPattern) ||
            !(parseInt(port) >= 5900 && parseInt(port) <= 5930)) {
                msg += gettext('Invalid Remote port.')+'\n';
        } else {
            for(var vm in InitInfo.vnc_used_ports){
                if (parseInt(port) == InitInfo.vnc_used_ports[vm]) {
                    msg += interpolate(gettext('Remote port %(vnc_used_ports)s have been used by [ %(vm)s ].'),{'vnc_used_ports':InitInfo.vnc_used_ports[vm],'vm':vm},true)+'\n';
                }
            }
        }
    }
    
    if (document.getElementById("adv_vnc_pwd_text").value) {
        ele = document.getElementById("adv_vnc_pwd_text");
        ele.value = ele.value.replace(/(^\s*|\s*$)/g, "");
		var re = /^([A-Za-z0-9_\-\.]+)$/g;
		if (!re.test(ele.value)) {
			msg += gettext('Invalid characters in remote password name. Please only use:') +'a-z,A-Z,0-9,_,-,.\n';
		}
	
	    if(ele.value != document.getElementById("adv_vnc_pwd_text_confirm").value){
		    msg += gettext('Password and confirmation password do not match')+'\n';
    	}
	
    }
    
    //check auto start with free memory
    if (document.getElementById("adv_autostart").checked) {
        if (InitInfo.autostart_cnt >= InitInfo.startvm_limit) {
            msg += gettext('Exceed maximun auto start num')+':'+InitInfo.startvm_limit+'\n';
        }
        if (parseInt(document.getElementById("adv_memory").value) > InitInfo.auto_free_mem) {
            msg += gettext('For auto start settings, memory must be smaller than ')+InitInfo.auto_free_mem+'MB\n';
        }
        
        ele = document.getElementById("adv_start_delay");
        ele.value = ele.value.replace(/(^\s*|\s*$)/g, "");
        var RegExPattern = /^[0-9]+$/;
        if (!ele.value.match(RegExPattern) ||
            !(parseInt(ele.value) >= 0 && parseInt(ele.value) <= 600)) {
                msg += gettext('Invalid startup delay time.')+'\n';
        }
    }

    //check mac
	var cnt = 0;
	var RegExPattern = /^([0-9a-fA-F]{1,2}[\.:-]){5}([0-9a-fA-F]{1,2})$/;
	while (document.getElementById('adv_nw_mac'+cnt)) {
		currentmac = document.getElementById('adv_nw_mac'+cnt).value;
		if (!currentmac.match(RegExPattern)) {
			msg += gettext('Invalid format of MAC')+' # '+ (cnt+1) + '\n';
		} else if (parseInt(currentmac.split(':')[0],16)%2){
		    msg += interpolate(gettext('First number of MAC #%(cnt)s must be even(ex.0,2,4,6,...)'),{'cnt':(cnt+1)},true)+'\n';
		}
		for(var cc=cnt-1;cc>=0;cc--)
        {
            if (document.getElementById('adv_nw_mac'+cc).value == currentmac) {
                msg += interpolate(gettext('MAC %(currentmac)s have been used by yourself.'),{'currentmac':currentmac},true)+'\n';
            }
        }
		cnt++;
	}
	
	//check network interface
	cnt = 0;
	while (document.getElementsByName('adv_nw_mode'+cnt).length > 0) {
		var mode = GetRadioBtnValue(document.getElementsByName('adv_nw_mode'+cnt));
		if (((mode == 'direct') && !document.getElementById('adv_eths_combos'+cnt).value) ||
		    ((mode == 'bridge') && !document.getElementById('adv_bridges_combos'+cnt).value)) {
		        msg += interpolate(gettext('Invalid interface mode for adapter %(cnt)s'),{'cnt':(cnt+1)},true)+'\n';
		}
		cnt++;
	}

	if (msg.length > 0) {
		alert(msg)
		return false;
	}

    return true;
}

function _callbk_before_advsubmit_check(http_request, elemBtn)
{
	if (http_request.readyState == 4) {
	    CloseMask();
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			if (!json.success) {
			    alert(json.errors);
			    return;
			}
			
			var msg = '';
			// check duplicate mac
    	    var cnt = 0;
    	    while (document.getElementById('adv_nw_mac'+cnt)) {
    	    	var currentmac = document.getElementById('adv_nw_mac'+cnt).value;
    	    	for (var i=0; i<json.used_macs.length; i++) {
            		if (currentmac == json.used_macs[i]) {
            		    msg += interpolate(gettext('MAC #%(cnt)s [ %(used_macs)s ] have been used.'),{'cnt':(cnt+1),'used_macs':json.used_macs[i]},true)+'\n';
            		    break;
            		}
        	    }
        	    cnt++;
        	}
        	if (msg.length > 0) {
        		alert(msg)
        		return false;
        	}
			elemBtn.click();	
		} else {
		    alert(gettext('File check failed.'));
		}
	}
}

function ShowHideCloningWarning(cloningVM)
{
    var elem = document.getElementById('cloning_warning');
    var elem2 = document.getElementById('vm_background_msg_'+cloningVM);
    elem.style.display = (cloningVM && !elem2) ? '' : 'none';
    elem.innerHTML = interpolate(gettext('Please wait to VM[%(cloningVM)s] cloning finish.'),{'cloningVM':cloningVM},true);
}

function alertExisting(vmName){
    alert(interpolate(gettext('The VM name [%(vmName)s] has been existing.'),{'vmName':vmName},true));
}
