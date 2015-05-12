NOVNC_PORT = 3388;
IMPORT_VM_EXTENSION_PC = [".ova", ".ovf", '.qvm', '.xml'];
IMPORT_VM_EXTENSION_NAS = [".ova", ".ovf", ".vmx", '.qvm', '.xml'];
PERMISSION_LIMIT_CONTROL = 2;
PERMISSION_LIMIT_VIEWONLY = 2;
		
function ShowMask(bShowCancel, elemID, blackMask) {
    if(document.getElementById("document-mask")){
	var o = document.getElementById("document-mask");
	o.style.visibility = "visible";
	o.style.zIndex = 10000;
	o.style.display = 'block';   
	o.style.position = 'absolute';   
	o.style.filter = "alpha(opacity:50)";   
	o.style.KHTMLOpacity = 0.8;   
	o.style.MozOpacity = 0.8;   
	o.style.opacity = 0.8;   
	o.style.background = '#FFF';
	o.style.height = window.innerHeight + 'px';
	
	if (blackMask){
		o.style.opacity = 0.6   
		o.style.background = '#000';	
	}
	
	if (elemID) {
	    var rect = document.getElementById(elemID).getBoundingClientRect();
	    //o.style.top = rect.top + 'px';
	    o.style.left = rect.left + 'px';
	    o.style.width = (window.innerWidth-rect.left) + "px"; 
	    document.getElementById("loading-icon").style.left = ((rect.right-rect.left)/2-32) + 'px';
	} else {
	    o.style.width = window.innerWidth + "px";
	    document.getElementById("loading-icon").style.left = (document.body.clientWidth/2-32) + 'px';
	}
	document.getElementById("loading-icon").style.top = ($(window).height()/2-32) + 'px';
	
	o = document.getElementById("loading-cancel");
	o.style.zIndex = 20000;
	o.style.display = 'block';   
	o.style.position = 'absolute';   
	o.style.height = document.getElementsByName('importvm_cancel')[0].clientHeight + 'px';
	if (elemID) {
	    var rect = document.getElementById(elemID).getBoundingClientRect();
	    o.style.left = (rect.left-15) + 'px';
	    o.style.width = (window.innerWidth-rect.left) + "px"; 
	} else {
	    o.style.width = window.innerWidth + "px";
	}
	document.getElementById("loading-cancel").style.top = (document.body.clientHeight/2+52) + 'px';
	document.getElementById("loading-cancel").style.visibility = bShowCancel ? 'visible':'hidden';
    }
}

function CloseMask() {
	var o = document.getElementById("document-mask");
	o.style.visibility = "hidden";
	document.getElementById("loading-cancel").style.visibility = 'hidden';
}
/*
function normalConfirm(msg)
{
    var ret = confirm(msg);
    if (ret) {
        ShowMask();
    }
    return ret;
}
*/
function SubmitConfirm(msg, msgTitle, submitID, sendValue, blackMask)
{
    jConfirm(msg, msgTitle, function(r) {
		if(r)
		{
		    if(submitID)
		    {
			$('#'+submitID).val(sendValue);
			$('#'+submitID).click();
		    }
		}
		else
		{
			if(!blackMask)
			CloseMask();
			return;
		}
	},blackMask);
	
	if(!blackMask)
	ShowMask(); 
}

window.alert = function(str,_callback, blackMask) {
	jAlert(str, '', _callback, blackMask);
}

function SubmitConfirmCustom(msg, msgTitle, formID, sendID, _call_backend)
{
    jConfirm(msg, msgTitle, function(r) {
		if(r)
		{
		    if(_call_backend)
		    {
		    	_call_backend();
		    }
		    else
		    {
			$('#'+sendID).attr("disabled", false);
		        $('#'+formID).submit();
		    }
		}
		else
			CloseMask();
	});
	ShowMask(); 
}

function wait(ms) 
{
    var start = +(new Date());
    while (new Date() - start < ms);
}

function makeRequest(url, data, func, param, sync) {
	var http_request = true;
	if (sync=="false")
		sync = false;
	else // (typeof sync == "undefined")
		sync = true;
	
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
	  http_request = new XMLHttpRequest();
	} else if (window.ActiveXObject) { // IE
	  try {
	    http_request = new ActiveXObject("Msxml2.XMLHTTP");
	  } catch (e) {
	    try {
	      http_request = new ActiveXObject("Microsoft.XMLHTTP");
	    } catch (e) {}
	  }
	}
	
	if (!http_request) {
	  alert('Giving up :( Cannot create an XMLHTTP instance');
	  return false;
	}
	
	http_request.onreadystatechange = function(){ func(http_request, param); }; 
	
	if(data==''){
		http_request.open('GET', url, sync);
		http_request.send(null);	
	}
	else{
		http_request.open('POST', url, sync);
		http_request.send(data);	
	}
}

function UpdateVMState(host_id, vname, vmstate)
{
	clearTimeout(VMStateTimer);

    var bExtendExpire = 0;
    if (typeof(bAutoLogin) !== "undefined" && bAutoLogin) {
        bExtendExpire = 1;
    }
	
	if($('#div_cpuus_text').length)		// Update cpu state
		UpdateVMHwState(host_id, vname, vmstate);
	url = "/vminfo/" + host_id + "/"+ vname +"/?bExtendExpire="+bExtendExpire+"&r=" + Math.random();
    makeRequest(url, '', _callbk_update_vmstate, {'host_id':host_id, 'vname':vname, 'vmstate':vmstate});    
}

function _callbk_update_vmstate(http_request, param)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			if ((!json.success && (json.errors.indexOf('Authentication failed') != -1)) || 
			    (param.vmstate != json.state)) {
                window.location.href = window.location.href
                return;
            }
		}
		VMStateTimer = setTimeout(function(){UpdateVMState(param.host_id, param.vname, param.vmstate);}, 5000);
	}
}

function UpdateVMHwState(host_id, vname, vmstate)
{
	url = "/vminfo/" + host_id + "/"+ vname +"/cpuus/?r=" + Math.random();
    makeRequest(url, '', _callbk_update_vm_cpuus, vmstate);
}

function _callbk_update_vm_cpuus(http_request, vmstate)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			var new_cpu_usg = parseInt(json.percentage,10);
			var old_cpu_usg = parseInt(document.getElementById('div_cpuus_text').innerHTML);
			if((new_cpu_usg > 10 && old_cpu_usg < 10) || (new_cpu_usg < 10 && old_cpu_usg > 10))
			{
				var str = '';
				if(new_cpu_usg > 10)
					str = '<span id="div_cpuus_bar" class="bar bblue" style="width:'+new_cpu_usg+'%;"><span id="div_cpuus_text" class="bar_ltext">'+new_cpu_usg+'%</span></span>';
				else
					str = '<span id="div_cpuus_bar" class="bar bblue" style="width:'+new_cpu_usg+'%;float:left;"></span><span id="div_cpuus_text" class="bar_rtext">'+new_cpu_usg+'%</span>';
				document.getElementById('div_cpuus').innerHTML = str;
			}
			else
			{
				document.getElementById('div_cpuus_bar').style.width = new_cpu_usg+'%';
				document.getElementById('div_cpuus_text').innerHTML = new_cpu_usg+'%';
			}
		}
	}
}

function UpdateTaskInfo(host_id)
{
	clearTimeout(TaskInfoTimer);
    makeRequest( "/taskinfo/"+host_id+'/', '', _callbk_update_taskinfo, host_id); 
}

function _callbk_update_taskinfo(http_request, host_id)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			if (!json.success && (json.errors.indexOf('Authentication failed') != -1)) {
				var refreshURL=window.location.href;
				refreshURL=refreshURL.substr(0,refreshURL.lastIndexOf('/'));
				refreshURL=refreshURL.substr(0,refreshURL.lastIndexOf('/'))+'/';
				window.location.href = refreshURL;
				return;
			}
            
			// show/hide export/import icon
			var elem = document.getElementById('task-import-icon');
			var title = "";
			if (elem) {
			    elem.style.display = (json.tasks.import.running || json.tasks.import.queuing) ? '' : 'none';
			    if (json.tasks.import.running) {
			        title += gettext('Running')+' : ' + json.tasks.import.running + '\n';
			    }
			    if (json.tasks.import.queuing) {
			        title += gettext('Queuing')+' : ' + json.tasks.import.queuing;
			    }
    			elem.title = title;

    		}
    		elem = document.getElementById('task-export-icon');
			title = "";
			if (elem) {
			    elem.style.display = (json.tasks.export.running || json.tasks.export.queuing) ? '' : 'none';
			    if (json.tasks.export.running) {
			        title += gettext('Running')+' : ' + json.tasks.export.running + '\n';
			    }
			    if (json.tasks.export.queuing) {
			        title += gettext('Queuing')+' : ' + json.tasks.export.queuing;
			    }
    			elem.title = title;
    		}	
    		
    		// show/hide vm background action icon
    		var cloningVM = '';
    		for (var i=0; i<json.vm_bkg.length; i++) {
    		    if (json.vm_bkg[i].lockmsg.indexOf('cloning') != -1) {
    		        cloningVM = json.vm_bkg[i].name;
    		    }
    		    
    		    elem = document.getElementById('vm-bkg-icon-'+json.vm_bkg[i].name);
    		    if (!elem) {
    		        continue;
    		    }
    		    elem.style.display = json.vm_bkg[i].lockmsg ? '' : 'none';
    		    elem.title = json.vm_bkg[i].lockmsg;
    		    
    		    elem = document.getElementById('vm_background_msg_'+json.vm_bkg[i].name);
        		if (elem) {
        		    elem.style.display = json.vm_bkg[i].lockmsg ? '' : 'none';
        		    elem.innerHTML = json.vm_bkg[i].lockmsg;
			    
    			    if(json.vm_bkg[i].lockmsg){
    				    $('#resumeBtn').attr('disabled',true);
    				    $('#virtio_iso_Btn').attr('disabled',true);
    				    $('#adv_addDeviceBtn').attr('disabled',true);
    				    $('#adv_rmDeviceBtn').attr('disabled',true);
    				    $('#adv_applyBtn').attr('disabled',true);			    
    			    }
        		}
    		}
    		
    		// show/hide warning message in cloning wizard
    		if (document.getElementById('cloning_warning')) {
    		    ShowHideCloningWarning(cloningVM)
    		}
		}
		TaskInfoTimer = setTimeout(function(){UpdateTaskInfo(host_id);}, 5000);
	}
}

function UpdateNetworkPlot(host_id, inf)
{
	clearTimeout(NWPlotTimer);
	
	url = "/nwinfo/" + host_id + "/" + inf + "/?r=" + Math.random()
    makeRequest(url, '', _callbk_update_network_plot);
    
    NWPlotTimer = setTimeout(function(){UpdateNetworkPlot(host_id,inf);}, 5000);
}

function _callbk_update_network_plot(http_request)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			for (var i=0; i<json.length; i++) {
				NetworkFlot.InsertData(json[i].bridge_name, json[i].rx, json[i].tx);
			}
			NetworkFlot.Repaint();
		}
	}
}

function CheckImportVMFile(ele, step)
{
	if (step == 1) {
		var msg = gettext('File type not allowed')+':\n';
		var bAlert = false;
		for (var f=0; f<ele.files.length; f++) {
			var str = ele.files[f].name.toLowerCase();
			var bValid = false;
			for (var i=0; i<IMPORT_VM_EXTENSION_PC.length; i++) {
				if (str.indexOf(IMPORT_VM_EXTENSION_PC[i], str.length - IMPORT_VM_EXTENSION_PC[i].length) != -1) {
					bValid = true;
					break;
		        }
		    }
		    if (!bValid) {
		    	bAlert = true;
		    	msg += ele.files[f].name + "\n";
		    }
		}
		
		if (bAlert) {
			alert(msg);
			ele.value = "";
		}
	} else if (step == 2) {
		var bAlert = false;
		var msg = gettext('Still missing the following files')+':\n';
		for (var i=0; i<RestFiles.length; i++) {
			var bValid = false;
			for (var f=0; f<ele.files.length; f++) {
				var str = ele.files[f].name;
				if (str.indexOf(RestFiles[i], str.length - RestFiles[i].length) != -1) {
					bValid = true;
					break;
		        }
			}
			if (!bValid) {
		    	bAlert = true;
		    	msg += '\t' + RestFiles[i] + "\n";
		    }
		}
		
		if (bAlert) {
			msg += gettext('Please re-select upload files!');
			alert(msg);
			ele.value = "";
		}
	}
	
    document.getElementById('bt_next').disabled = (ele.files.length > 0) ? false : true;
}

function GetScrollbarWidth() 
{
    var parent, child, width;
    if (width === undefined) {
        parent = $('<div style="width:50px;height:50px;overflow:auto"><div/></div>').appendTo('body');
        child = parent.children();
        width = child.innerWidth() - child.height(99).innerWidth();
        parent.remove();
    }
    return width;
}

function MacGenerator(inputID)
{
    var mac = "52:54:00"
    for (var i=0; i<3; i++) {
        var tmp = Math.floor(Math.random()*256).toString(16).toUpperCase();
        if (tmp.length < 2) {
            tmp = '0' + tmp;
        }
        mac += ':' + tmp; 
    }

    $('#'+inputID).css({'background-color':'','border':'', 'box-shadow': ''});
    $('#'+inputID).attr('title','');    
    
    return mac;
}

function CreateMemSliderElement(elem_id)
{
    var link = [256, MAX_MEMORY_VALUE/4, MAX_MEMORY_VALUE/2, MAX_MEMORY_VALUE*3/4, MAX_MEMORY_VALUE];
    if (MAX_MEMORY_VALUE == 512){
	link = [256, 512];
    }
    else if (MAX_MEMORY_VALUE == 768){
	link = [256, 512, 768];
    }
    else if (MAX_MEMORY_VALUE == 1024){
	link = [256, 512, 768, 1024];
    }

    var str = '';
    for (var i=0; i<link.length; i++) {
        var str2 = (link[i] < 1024) ? (link[i]+' MB') : ((link[i]/1024)+' GB');
        var textalign = 'center';
        var percent = 100/(link.length-1);
        if (i == (link.length-1)) {
            percent /= 2;
            textalign = 'right';
        }else if (i == 0) {
            textalign = 'left';
            percent /= 2;
        }
        str += '<a href=# style="width:'+percent+'%;display:inline-block;text-align:'+textalign+';" onclick="UpdateMemorySlider(\''+elem_id+'\', '+link[i]+');">'+str2+'</a>';
    }
    str += '<div id="slider-range-min" style="margin-bottom:10px"></div>';
    
    document.getElementById("memory_slider_link").innerHTML = str;
}

function UpdateMemorySlider(elemID, value)
{
    var reg = /^[0-9]*$/;
    if (!reg.test(value)) {
        value = $("#slider-range-min").slider("value");
    }
    
    if (value < 256) {
        value = 256;
    } else if (value > MAX_MEMORY_VALUE) {
        value = MAX_MEMORY_VALUE;
    }
    
    $("#slider-range-min").slider("value", value);
    if (document.getElementById(elemID).value != value) {
        document.getElementById(elemID).value = value;
    }
}

function OnHDDSourceChange(value, id_prefix, total)
{
    for (var i=0; i<=total;i++) {
        document.getElementById(id_prefix+i).className = (value == i) ? "active" : "";
    }
    
    var eleTrs = document.getElementById("hdd_table").getElementsByTagName('tr');
    var cnt = 0;
	for (var i=0; i<eleTrs.length;i++) {
	    var html = eleTrs[i].innerHTML;;
	    if (html.indexOf(STR_SOURCE_PATH) != -1) {
	        cnt++;
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("source_path_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    } else if (html.indexOf(STR_FORMAT) != -1) {
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("format_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    } else if (html.indexOf(STR_CACHE_MODE) != -1) {
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("cache_mode_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    } else if (html.indexOf(STR_CONTROLLER) != -1) {
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("controller_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    }
	}
}

function OnBridgeChange(value, id_prefix, total)
{
    for (var i=0; i<=total;i++) {
        document.getElementById(id_prefix+i).className = (value == i) ? "active" : "";
    }
    
    var eleTrs = document.getElementById("network_table").getElementsByTagName('tr');
    var cnt = 0;
	for (var i=0; i<eleTrs.length;i++) {
	    var html = eleTrs[i].innerHTML;
	    if (html.indexOf(STR_MODE) != -1) {
	        cnt++;
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("bridge_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    } else if (html.indexOf(STR_MAC_ADDRESS) != -1) {
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("mac_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    } else if (html.indexOf(STR_DEVICE_MODEL) != -1) {
	        eleTrs[i].style.display = ((value == 0) || (value == cnt)) ? "" : "none";
	        document.getElementById("device_moidel_label_"+cnt).style.display = (value == 0) ? "inline" : "none";
	    }
	}
}

function SetRadioBtnValue(radioObj, newValue) {
    if  (!radioObj) {
		return;
	}
	var radioLength = radioObj.length;
	if (radioLength == undefined) {
		radioObj.checked = (radioObj.value == newValue.toString());
		return;
	}
	for (var i=0; i<radioLength; i++) {
		radioObj[i].checked = false;
		if(radioObj[i].value == newValue.toString()) {
			radioObj[i].checked = true;
		}
	}
}

function GetRadioBtnValue(radioObj) {
	if (!radioObj) {
		return "";
	}
	var radioLength = radioObj.length;
	if (radioLength == undefined) {
		if (radioObj.checked) {
			return radioObj.value;
		} else {
			return "";
		}
	}
	for (var i = 0; i < radioLength; i++) {
		if (radioObj[i].checked) {
			return radioObj[i].value;
		}
	}
	return "";
}

function detectBrowser(){
	var sAgent = navigator.userAgent.toLowerCase();
	this.isIE = (sAgent.indexOf("msie")!=-1); //IE6.0-7
	this.isFF = (sAgent.indexOf("firefox")!=-1);//firefox
	this.isSa = (sAgent.indexOf("safari")!=-1);//safari
	this.isOp = (sAgent.indexOf("opera")!=-1);//opera
	this.isNN = (sAgent.indexOf("netscape")!=-1);//netscape
	this.isCh = (sAgent.indexOf("chrome")!=-1);//chrome
	this.isMa = this.isIE;//marthon
	this.isOther = (!this.isIE && !this.isFF && !this.isSa && !this.isOp && !this.isNN && !this.isSa);//unknown Browser
	
	this.isMobile = false;
	if (sAgent.match(/(iphone|ipod|ipad|android)/)){
		this.isMobile = true;
	}	
}

var oBrowser = new detectBrowser();
function resizeRightDiv()
{
	if (oBrowser.isCh) { 
		vWidth = window.innerWidth-265-20+23;
		vHeight = window.innerHeight-20;
		vContainHeight = window.innerHeight-20-10;
		vMenuHeight = window.innerHeight-2-8+4;
		document.getElementById('rightDiv').style.padding="10px 10px 0";
		document.getElementById('rightContainDiv').style.margin="0";
	}
	else { 
		vWidth = window.innerWidth-265-20+23;
		vHeight = window.innerHeight-20-10;
		vContainHeight = window.innerHeight-20-10;
		vMenuHeight = window.innerHeight-2+4;
	}
	document.getElementById('rightDiv').style.width = parseInt(vWidth)+'px';
	document.getElementById('rightDiv').style.height = parseInt(vHeight)+'px';
	if(document.getElementById('rightContainDiv').scrollHeight < parseInt(vHeight))
		document.getElementById('rightContainDiv').style.height = parseInt(vContainHeight)+'px';
	else
		document.getElementById('rightContainDiv').style.height = 'auto';
	document.getElementById('leftDiv').style.height = parseInt(vMenuHeight)+'px';
}

function CreateHDDSliderElement(elem_id,startSize,endSize,style)
{
	startSize=parseInt(startSize);
	endSize=parseInt(endSize);

    var level_1=1024;
    var level_2=2048;
    var level_3=3072;
    var sliderWidth=202;
    link = [startSize, level_1, level_2, level_3, endSize];

	if(style=='exMode')
	{
	    if(startSize>=level_3)
	    {
		link = [level_3, endSize];
	    }
	    else if(startSize>=level_2)
	    {
		link = [level_2, level_3, endSize];
	    }
	    else if(startSize>=level_1)
	    {
		link = [level_1, level_2, level_3, endSize];
	    }	    
	    else
	    {
		link = [0, level_1, level_2, level_3, endSize];
	    }
	    sliderWidth=305;
	}
	else
	{
	    style='others';
	}
    
    var str = '';
    
    for (var i=0; i<link.length; i++) {
        var str2 = link[i];
        var textalign = 'center';
        var percent = 100/(link.length-1);
        if (i == (link.length-1)) {
            percent /= 2;
            textalign = 'right';
	    str2='';
        }else if (i == 0) {
            textalign = 'left';
            percent /= 2;
        }
	
	if(link[i]<startSize)
	{
		str += '<span style="width:'+percent+'%;display:inline-block;text-align:'+textalign+';">'+str2+'</span>';
	}
        else
	{
        str += '<a href=# style="width:'+percent+'%;display:inline-block;text-align:'+textalign+';" onclick="UpdateHDDSlider(\''+elem_id+'\', '+link[i]+', '+startSize+', '+endSize+');">'+str2+'</a>';
    }
    }
    
    str += '<div style="width:35px;margin-top:-20px;margin-left:'+sliderWidth+'px;"><a href=# style="width:'+percent+'%;display:inline-block;text-align:'+textalign+';" onclick="UpdateHDDSlider(\''+elem_id+'\', '+link[link.length-1]+', '+startSize+', '+endSize+');">'+link[link.length-1]+'</a></div>';
    str += '<div id="slider-range-HDDmin" style="margin-bottom:10px"></div>';
    
    document.getElementById("HDD_slider_link").innerHTML = str;
}

function UpdateHDDSlider(elemID, value, startSize, endSize, style)
{
    var reg = /^[0-9]*$/;
    if (!reg.test(value)) {
        value = $("#slider-range-HDDmin").slider("value");
    }
    
    if (value < startSize) {
        value = startSize;
    } else if (value > endSize) {
        value = endSize;
    }
    
    $("#slider-range-HDDmin").slider("value", value);
    if (document.getElementById(elemID).value != value) {
        document.getElementById(elemID).value = value;
    }
    
    if(style=='exMode'){
	document.getElementById("Hddresize_id").value = value-startSize;
    }
    	
}

function FL_CreateHDDSliderElement(elem_id,startSize,endSize)
{
	startSize=parseInt(startSize);
	endSize=parseInt(endSize);
    var sizeRange=endSize-startSize;
    var level_1=1024;
    var level_2=2048;
    var level_3=3072;   
/*    
    var level_1=Math.round((startSize+sizeRange/4)/50)*50;
    var level_2=Math.round((startSize+sizeRange/2)/50)*50;
    var level_3=Math.round((startSize+sizeRange*3/4)/50)*50;
*/    
    var link = [startSize, level_1, level_2, level_3, endSize];
    var str = '';
    for (var i=0; i<link.length; i++) {
        var str2 = link[i];
        var textalign = 'center';
        var percent = 100/(link.length-1);
        if (i == (link.length-1)) {
            percent /= 2;
            textalign = 'right';
	    str2='';
        }else if (i == 0) {
            textalign = 'left';
            percent /= 2;
        }
        str += '<a href=# style="width:'+percent+'%;display:inline-block;text-align:'+textalign+';" onclick="FL_UpdateHDDSlider(\''+elem_id+'\', '+link[i]+', '+startSize+', '+endSize+');">'+str2+'</a>';
    }
    
    str += '<div style="width:35px;margin-top:-20px;margin-left:202px;"><a href=# style="display:inline-block;text-align:'+textalign+';" onclick="FL_UpdateHDDSlider(\''+elem_id+'\', '+link[link.length-1]+', '+startSize+', '+endSize+');">'+link[link.length-1]+'</a></div>';
    str += '<div id="FL_slider-range-HDDmin" style="margin-bottom:10px"></div>';

    document.getElementById("FL_HDD_slider_link").innerHTML = str;
}

function FL_UpdateHDDSlider(elemID, value, startSize, endSize)
{
    var reg = /^[0-9]*$/;
    if (!reg.test(value)) {
        value = $("#FL_slider-range-HDDmin").slider("value");
    }

    if (value < startSize) {
        value = startSize;
    } else if (value > endSize) {
        value = endSize;
    }

    $("#FL_slider-range-HDDmin").slider("value", value);
    if (document.getElementById(elemID).value != value) {
        document.getElementById(elemID).value = value;
    }

}

function ReloadShareList(func, params)
{
    url = "/sharelist/";
    makeRequest(url, '', _callbk_reload_sharelist, {'func':func, 'params':params});
}

function _callbk_reload_sharelist(http_request, data)
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
            if (json.success) {
                if (data.params) {
                    data.func(json.list_data, data.params);
                } else {
                    data.func(json.list_data);
                }
            } else {
                func(null, []);
            }
		} else {
		    func(null, []);
		}
	}
    
}

function decodeHTMLEntities(text) {
    var entities = [
        ['apos', '\''],
        ['amp', '&'],
        ['lt', '<'],
        ['gt', '>']
    ];

    for (var i = 0, max = entities.length; i < max; ++i) {
        text = text.replace(new RegExp('&'+entities[i][0]+';', 'g'), entities[i][1]);
    }

    return text;
}

function FileSizeFormat(bytes) {
    var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    if (bytes == 0) return 'n/a';
    var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
    return (bytes / Math.pow(1024, i)).toFixed(1) + ' ' + sizes[i];
};

function onMultiCkboxChange(elem, prefix, totalCnt,openBtn)
{
    if (elem.id == (prefix+'all')) {
        for (var i=0; i<totalCnt; i++) {
            if (!document.getElementById(prefix+i).disabled) {
                document.getElementById(prefix+i).checked = elem.checked;
            }
        }

	if(openBtn){
		if(elem.checked)
			document.getElementById(openBtn).disabled = false;
		else
			document.getElementById(openBtn).disabled = true;
	}
	
    } else {
        var bSelectAll = true;
	var isSelect = false;
        for (var i=0; i<totalCnt; i++) {
            if (!document.getElementById(prefix+i).disabled && !document.getElementById(prefix+i).checked) {
                bSelectAll = false;
                //break;
            }
	    
	    if(document.getElementById(prefix+i).checked)
		isSelect = true;
            }
        document.getElementById(prefix+'all').checked = bSelectAll;
	
	if(openBtn){
		if(isSelect)
			document.getElementById(openBtn).disabled = false;
		else
			document.getElementById(openBtn).disabled = true;
        }
    }
}

function createHelpTip(helpContent,baseWidth){
	var helpTip = $('<div/>').attr('id','helpTip').addClass('helpTip').appendTo($('#rightDiv'));
	var helpTipBody = $('<div/>').addClass('helpTipBody').css({'width':baseWidth}).appendTo(helpTip);
	var helpTipHeadBg = $('<div/>').addClass('helpTipHeadBg').appendTo(helpTip);
	var helpTipHead = $('<div/>').addClass('helpTipHead').appendTo(helpTip);	
	
	$(helpContent).appendTo(helpTipBody);

	var detectUserAgent = navigator.userAgent;
	if (detectUserAgent.match(/(iPhone|iPod|iPad|Android)/)){
		$('#help-window').on('touchstart click',function(e){
			e.preventDefault();
			var rect = document.getElementById('help-window').getBoundingClientRect();
			var baseLeft=rect.left+11;
			var baseTop=rect.top+30;
			helpTipBody.css({'left':baseLeft-baseWidth,'top':baseTop});
			helpTipHeadBg.css({'left':baseLeft-5,'top':baseTop-6});
			helpTipHead.css({'left':baseLeft-5,'top':baseTop-5});		
			helpTip.show();
			helpTip.before($('<div id="closeMask">'));
			$('#closeMask').on('touchstart click',closeTip);
		});
				
	}
	else{
		$('#help-window').mouseover(function(){
			var rect = document.getElementById('help-window').getBoundingClientRect();
			var baseLeft=rect.left+11;
			var baseTop=rect.top+30;
			helpTipBody.css({'left':baseLeft-baseWidth,'top':baseTop});
			helpTipHeadBg.css({'left':baseLeft-5,'top':baseTop-6});
			helpTipHead.css({'left':baseLeft-5,'top':baseTop-5});		
			helpTip.show();
			
			$('#help-window').click(function(e){
				if(!$('#closeMask').attr('id'))
					helpTip.before($('<div id="closeMask">'));
				$('#help-window').off('mouseout');
				$('#closeMask').on('click',closeTip);				
		});
	
		$('#help-window').mouseout(function(){
			$('.helpTip').hide();
		});	

			function closeTip(e){
				e.preventDefault();
				$('.helpTip').hide();
				$('#closeMask').remove();
			}			
			
		});
	}
}

function closeTip(e){
	e.preventDefault();
	$('.helpTip').hide();
	$('#closeMask').remove();
}

//account function start
function accountMenu(userName,lastLoginTime,page){
	if($('#accountMenu').attr('id')!=undefined){
		$('#accountMenu').remove();
		$('#closeMask').remove();
	}
	else{
		var rect = $('.account')[0].getBoundingClientRect();
		var baseTop=rect.top+28;	
		var scrollWidth=18;
		
		if (oBrowser.isCh){
			scrollWidth=4;
		}
		
		if($('#rightDiv')[0].scrollHeight > $('#rightDiv')[0].clientHeight){
			scrollWidth=0;
		}

		if(page=='vm'){
			baseTop = 60;
		}		

		$('<div id="closeMask">').appendTo($('#rightDiv'));
		var accountMenu = $('<div id="accountMenu"/>').appendTo($('#rightDiv'));
		
		if (oBrowser.isCh){
			var accountMenuContent = $('<div/>').addClass('accountContent').css({'right':15-scrollWidth,'top':baseTop}).appendTo(accountMenu);
			var accountHeadBg = $('<div/>').addClass('accountHeadBg').css({'right':33-scrollWidth,'top':baseTop-7}).appendTo(accountMenu);
			var accountHead = $('<div/>').addClass('accountHead').css({'right':33-scrollWidth,'top':baseTop-5}).appendTo(accountMenu);		
		}
		else{
			var accountMenuContent = $('<div/>').addClass('accountContent').css({'right':30-scrollWidth,'top':baseTop}).appendTo(accountMenu);
			var accountHeadBg = $('<div/>').addClass('accountHeadBg').css({'right':48-scrollWidth,'top':baseTop-7}).appendTo(accountMenu);
			var accountHead = $('<div/>').addClass('accountHead').css({'right':48-scrollWidth,'top':baseTop-5}).appendTo(accountMenu);
		}

		if(userName=='admin'){
			if(window.parent != window.self){
				accountMenuContent.append('<div class="accountHr" style="border-bottom:1px solid #B2B2B2;"><div class="showTime" style="margin-top:5px"><b>'+userName+'</b><br>'+gettext('Last login time')+': </div>'+
						'<div class="showTime">'+lastLoginTime+'</div></div>'+
						'<a data-toggle="modal" href="#aboutQVS" style="text-decoration:none;"><div class="accountBtn accountLastBtn" onclick="accountControl()"><div>'+gettext('About')+'</div></div></a>');
			}
			else{
				accountMenuContent.append('<div class="accountHr" style="border-bottom:1px solid #B2B2B2;"><div class="showTime" style="margin-top:5px"><b>'+userName+'</b><br>'+gettext('Last login time')+': </div>'+
						'<div class="showTime">'+lastLoginTime+'</div></div>'+
						'<div class="accountBtn" onclick="accountControl(\'logout\')"><div>'+gettext('Logout')+'</div></div>'+
						'<a data-toggle="modal" href="#aboutQVS" style="text-decoration:none;"><div class="accountBtn accountLastBtn" onclick="accountControl()"><div>'+gettext('About')+'</div></div></a>');
			}
		}
		else{
			accountMenuContent.append('<div class="accountHr" style="border-bottom:1px solid #B2B2B2;"><div class="showTime" style="margin-top:5px"><b>'+userName+'</b><br>'+gettext('Last login time')+': </div>'+
					'<div class="showTime">'+lastLoginTime+'</div></div>'+
					'<a data-toggle="modal" href="#changePassword" style="text-decoration:none;"><div class="accountBtn" onclick="accountControl()"><div>'+gettext('Change Password')+'</div></div></a>'+
					'<div class="accountBtn" onclick="accountControl(\'logout\')"><div>'+gettext('Logout')+'</div></div>'+
					'<a data-toggle="modal" href="#aboutQVS" style="text-decoration:none;"><div class="accountBtn accountLastBtn" onclick="accountControl()"><div>'+gettext('About')+'</div></div></a>');
		}
	
		$('#closeMask').on('touchstart click',closeAccount);
	}
}

function closeAccount(e){
	e.preventDefault();
	$('#accountMenu').remove();
	$('#closeMask').remove();
}

function accountControl(choice,blackMask){
	$('.warningStr').hide();
	$('#closeMask').remove();
	$('#accountMenu').remove();

	if(choice=='logout'){
		ShowMask();
		jConfirm(gettext('Are you sure to log out?'), gettext('Logout Confirm'), function(r) {
			if(r)
			{	
				document.location.href='/admin/logout/';
			}
			else
			{	
				CloseMask();
				return;
			}		
		},blackMask);
	}
}
//account function end

function getCookie(key)
{
	if( document.cookie.length==0 )
		return false;
	
	var i=document.cookie.search(key+'=');
	if( i==-1 )
		return false;
	
	i+=key.length+1;
	var j=document.cookie.indexOf(';', i);
	if( j==-1 )
		j=document.cookie.length;
	return document.cookie.slice(i,j);
}

function fireEvent(element, event)
{
    if (document.createEventObject) {
        // dispatch for IE
        var evt = document.createEventObject();
        return element.fireEvent('on'+event,evt)
    } else {
    // dispatch for firefox + others
        var evt = document.createEvent("HTMLEvents");
        evt.initEvent(event, true, true ); // event type,bubbling,cancelable
        return !element.dispatchEvent(evt);
    }
}

function OnVNCShapshot(host_id, vmname, elem)
{
	url = "/vncsnapshot/"+host_id+"/"+vmname+"/?r=" + Math.random();
    makeRequest(url, '', _callbk_vnc_snapshot, elem.id);
}

function _callbk_vnc_snapshot(http_request, elemID)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			console.log(json);
			if (!json.success) {
				console.log('[Failed]'+json.errors)
				return;
			}
			// update image source path = '/media/img/'+json.vname+'_snapshot.jpg'
			showVNCSnapshot(json.vname);			
		}
	}
}

function creatVNCSnapshot(host_id){
	var helpTip = $('<div/>').attr('id','vncSnapshot').addClass('helpTip').appendTo($('#rightDiv'));
	var helpTipBody = $('<div/>').attr('id','vncSnapshotBody').addClass('helpTipBody').appendTo(helpTip);
	var helpTipHeadBg = $('<div/>').attr('id','vncSnapshotHeadBg').addClass('helpTipHeadBg').appendTo(helpTip);
	var helpTipHead = $('<div/>').attr('id','vncSnapshotHead').addClass('helpTipHead').appendTo(helpTip);
	
	$('.OnVNCShapshot').on('mouseover',function(e){
		e.preventDefault();
		var vmname = $(this).attr('id').replace('vnc_','');

		if(oBrowser.isFF){
			OnVNCShapshot(host_id, vmname, $(this));
		}
		else if(oBrowser.isCh){
			if(e.relatedTarget.id == $(this).attr('id'))
				OnVNCShapshot(host_id, vmname, $(this));		
		}
		else{
			var sAgent = navigator.userAgent.toLowerCase()
			isWin8 = (sAgent.indexOf("windows nt 6.2")!=-1);

			if(isWin8){
				OnVNCShapshot(host_id, vmname, $(this));
			}
			else{
				if(e.relatedTarget.id == $(this).attr('id'))
					OnVNCShapshot(host_id, vmname, $(this));		
			}
		}
		
		$('#rightDiv').on('touchstart click',function(e){
			e.preventDefault();
			$('#vncSnapshot').hide();
		});		
	});
	
	$('.OnVNCShapshot').mouseout(function(e){
		e.preventDefault();
		$('#vncSnapshot').hide();
	});	
}

function showVNCSnapshot(vname){
	$('#vncSnapshotBody').children().remove();
	var baseWidth = 400;
	var baseHeight = 250;
	var dt=new Date();
	var imgPath = '/media/img/'+vname+'_snapshot.jpg?r='+dt.getTime();
	$('<img/>').attr('src',imgPath).css('height',250).appendTo($('#vncSnapshotBody'));
	
	var detectUserAgent = navigator.userAgent;
	if($('#btn_console').attr('id')==undefined){					//overview page
		$('#vncSnapshotBody').css({'height':baseHeight});
	
		var winWidth = window.innerWidth;
		var rect = document.getElementById('vnc_'+vname).getBoundingClientRect();
		var baseLeft=rect.left+11;
		var baseTop=rect.top-5;
		
		if((baseTop-272)>0){
			$('#vncSnapshotHeadBg').removeClass().addClass('vncSnapshotHeadBg');
			$('#vncSnapshotHead').removeClass().addClass('vncSnapshotHead');
			$('#vncSnapshotBody').css({'right':winWidth-baseLeft-10,'top':baseTop-272});
			$('#vncSnapshotHeadBg').css({'left':baseLeft-10,'top':baseTop-5});
			$('#vncSnapshotHead').css({'left':baseLeft-10,'top':baseTop-6});						
		}
		else{
			$('#vncSnapshotHeadBg').removeClass().addClass('helpTipHeadBg');
			$('#vncSnapshotHead').removeClass().addClass('helpTipHead');			
			baseTop=rect.top+30;
			$('#vncSnapshotBody').css({'right':winWidth-baseLeft-10,'top':baseTop});
			$('#vncSnapshotHeadBg').css({'left':baseLeft-10,'top':baseTop-6});
			$('#vncSnapshotHead').css({'left':baseLeft-10,'top':baseTop-5});			
		}
		
		$('#vncSnapshot').show();
	
	}
	else{										//VM page
		//$('#vncSnapshotBody').css({'width':baseWidth});
		$('#vncSnapshotHeadBg').addClass('helpTipHeadBg');
		$('#vncSnapshotHead').addClass('helpTipHead');	

		var windowHeight = window.innerHeight;
		var rect = document.getElementById('btn_console').getBoundingClientRect();
		var baseLeft=rect.left+11;
		var baseTop=rect.top+30;
		
		$('#vncSnapshotBody').css({'left':baseLeft+20,'top':baseTop+7});
		$('#vncSnapshotHeadBg').css({'left':baseLeft-5+40,'top':baseTop-6+5});
		$('#vncSnapshotHead').css({'left':baseLeft-5+40,'top':baseTop-5+5});		
		
		if($('#conselModal').hasClass('in')){
			$('#vncSnapshot').hide();
		}
		else{
			$('#vncSnapshot').show();
		}
		
		$('#btn_console').on('mouseout click',function(){
			$('#vncSnapshot').hide();
		});	

		$('body').click(function(){
			$('#vncSnapshot').hide();
		});
			
		
	}
}

function addSSLHint(){
	var targetVNC = window.location.protocol + "//" + window.location.hostname + ":" + WebUtil.getQueryVar('port', '');
	var SSLbrowser = "FF";
	var hint = "";

	$('#noVNC_canvas').hide();
	$('body').css('overflow','auto');
	if(oBrowser.isFF){
		SSLbrowser = "FF";
		hint = 	'<div class="hintTitle">'+gettext("Please add the console's URL to your web browser's [Exception List].")+'</div>'+
			'<div class="hintText"><span class="hintNum">1.</span>'+gettext('Please click <button id="hintBtn01">start</button> to open a new tab.')+'</div>'+
			'<div><img src=..\\..\\..\\media\\img\\addSSLHint\\'+SSLbrowser+'\\en.jpg></div>'+
			'<div class="hintText"><span class="hintNum">4.</span>'+gettext('Close the new tab')+'</div>'+
			'<div class="hintText"><span class="hintNum">5.</span>'+gettext('Please click <button id="hintBtn02">refresh</button> to refresh this page.')+'</div>';
		$('<div>').attr('id','addSSLHint').appendTo($('#noVNC_container'));
	}
	else if(oBrowser.isIE){
		SSLbrowser = "IE";
		hint = 	'<div class="hintTitle">'+gettext('For security concerns, the HTML5 Console with SSL does not support Internet Explorer.')+'</div>'+
			'<div class="hintText"><center>'+gettext('Click [Redirect] button to open a normal console page.')+'</center></div>'+
			'<div class="hintText"><center><button id="hintBtn03">'+gettext('Redirect')+'</button></center></div>';
		$('<div>').attr('id','addSSLHint').css({'height':'150px','width':'800px','margin-left':'-400px','padding':'40px 20px 0 20px'}).appendTo($('#noVNC_container'));
	}
	
	$(hint).appendTo($('#addSSLHint'));
	
	$('#hintBtn01').on('touchstart click',function(){
		window.open(targetVNC, '_blank');
	});
	
	$('#hintBtn02').on('touchstart click',function(){
		location=location;
	});	
	
	$('#hintBtn03').on('touchstart click',function(){
		var port = window.location.port;
		var url = window.location.href.replace('https:','http:');
		var r1 = ':'+port+'/';
		var r2 = ':'+(port-1)+'/';
		url = url.replace(r1, r2);
		location=url;
	});	
}