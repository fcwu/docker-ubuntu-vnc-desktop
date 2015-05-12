var gNodeIdCount = 100;
DEFAULT_POOL_FOLDER_ID = "";

MODE_SINGLE_FOLDER = 1;
MODE_SINGLE_FILE = 2;
FILTER_FOLDER = 'folder';
FILTER_EXPORT = 'export';
FILTER_ISO = 'iso';
FILTER_IMG = 'img';


function QFolderTree(mode, filter)
{	
	//public methods
	QFolderTree.prototype.initTree = initTree;
	QFolderTree.prototype.destroy = destroy;
	QFolderTree.prototype.expandAll = expandAll;
	QFolderTree.prototype.collapseAll = collapseAll;
	QFolderTree.prototype.onItemlClick = onItemlClick;
	QFolderTree.prototype.getItemInfo = getItemInfo;
	QFolderTree.prototype.selectFolder = selectFolder;
	
	this.FolderList = [];
	this.FocusID = "";
	this.Mode = mode ? mode : MODE_SINGLE_FOLDER;
	this.Filter = filter ? filter : FILTER_FOLDER;
	this.bInit = false;
	this.BodyObgID = '';
	this.argDefaultFolders = [];
	this.bAutoSelect = false;
	
	var idTreeViewRoot = 'treeviewroot';
	var idTreeViewControl = 'treeviewcontrol';
	var idTreeViewCollapse = 'treeviewcollapse';
	var idTreeViewExpand = 'treeviewexpand';

	function initTree(bodyObjID, shareList, rootName) {
	    gNodeIdCount = 100;
	    this.FolderList = shareList;
	    this.BodyObgID = bodyObjID;
	    
		var tmpStr = '<div id="'+idTreeViewControl+'"><a href="?#" id="'+idTreeViewCollapse+'">Collapse All</a> | <a href="?#" id="'+idTreeViewExpand+'">Expand All</a></div>' +
		    '<ul id="'+idTreeViewRoot+'" class="filetree treeview-famfamfam">'+
		    '<li><span id="node_'+(gNodeIdCount++)+'" class="folder" style="background-image: url(\'../../../media/img/treeview/nas.png\');background-position:0 3px;padding-left:22px;">'+rootName+'</span><ul>';
		
		for (var i=0; i<this.FolderList.length; i++) {  
		    this.FolderList[i].id = 'node_' + gNodeIdCount;
		    if (this.FolderList[i].name == 'Public') {
		        this.FocusID = this.FolderList[i].id; 
		        DEFAULT_POOL_FOLDER_ID = this.FolderList[i].id;
		    }
					
		    tmpStr += '<li><span id="node_'+gNodeIdCount+'" class="'+this.FolderList[i].type+'">'+this.FolderList[i].name+'</span><ul id="root_node_'+(gNodeIdCount++)+'"  style="display:none;"></ul></li>';
		}
		
		tmpStr += '</ul></li></ul>';
		document.getElementById(bodyObjID).innerHTML = tmpStr;
        
        var parentElm = this;
		$('#'+idTreeViewRoot).treeview({
    	    control:'#'+idTreeViewControl,
    		toggle: function() {
    		    var elemID =  $(this).find(">span").attr('id');
    		    if (!parentElm.bInit || elemID == 'node_100') {
    		        return false;
    		    }
    		    parentElm.onItemlClick(elemID);
    		}
    	});
    	this.collapseAll();
    	this.bInit = true;
	}
	
	function destroy() {
	    if (this.BodyObgID) {
	        document.getElementById(this.BodyObgID).innerHTML = '';
	    }
	    this.FolderList = [];
	}
	
	function expandAll() {
	    document.getElementById(idTreeViewExpand).click();
	}
	
	function collapseAll() {
	    document.getElementById(idTreeViewCollapse).click();
	    document.getElementById('node_100').click();
	}
	
	function getItemInfo(elemID) {
	    var item = null;
	    for (var i=0; i<this.FolderList.length; i++) {
	        if (this.FolderList[i].id == elemID) {
	            item = this.FolderList[i];
	            break;
	        }
	    }
	    return item;
	}
	
	function onItemlClick(elemID) {
	    var item = this.getItemInfo(elemID);
	    if (!item) {
	        return;
	    }
	    
	    if (item.type == 'file') {
	        UpdateFocusFile(item.id); // implement by owner page
		} else {
		    UpdateFocusFolder(item.id); // implement by owner page
		}
			
	    if (item.load || (item.type != 'folder')) {
	        return;
	    }
	    
	    var url = "/folderlist/?path=" + item.fullpath + "&filter=" + this.Filter + "&id=" + item.id;
        makeRequest(url, '', _callbk_load_folder_list, this); 
	}
	
	function selectFolder(folders) {
	    if (folders.length < 2) {
	        return;
	    }
	    
	    this.argDefaultFolders = folders
	    this.bAutoSelect = true;
	    
	    var find_cnt = 0;
	    var cur_path = '/' + folders[0] + '/' + folders[1] + '/';
	    for (var t=2;t<folders.length;t++) {
	        cur_path += folders[t] + '/';
	        for (var i=0; i<this.FolderList.length; i++) {
    	        if (this.FolderList[i].fullpath == cur_path) {
    	            find_cnt = t;
    	            if (!this.FolderList[i].load) {
    	                document.getElementById(this.FolderList[i].id).click();
    	            }
    	            break;
    	        }
    	    } 
	    }
	    
	    if (find_cnt == (folders.length-1)) {
	        this.bAutoSelect = false;
	    }
	}
}

function _callbk_load_folder_list(http_request, objTree)
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			var json = JSON.parse(http_request.responseText);
			if (!json.success) {
			    window.location.href = window.location.href
			    return;
			}
			
			var item = null;
    	    for (var i=0; i<objTree.FolderList.length; i++) {
    	        if (objTree.FolderList[i].id == json.id) {
    	            objTree.FolderList[i].load = true;
    	            item = objTree.FolderList[i];
    	            break;
    	        }
    	    }
    	    if (!item) {
    	        return;
    	    }
			
			var tmpStr = '';
			
			// list folder items 
			for (var i=0; i<json.list_data.length; i++) {
			    if (json.list_data[i].type != 'folder') {
			        continue;
			    }
			    
			    var newItem = json.list_data[i];
			    newItem.id =  'node_' + gNodeIdCount;
			    FileFolderTree.FolderList.push(newItem);
		        tmpStr += '<li><span id="node_'+gNodeIdCount+'" class="'+newItem.type+'">'+newItem.name+'</span><ul id="root_node_'+gNodeIdCount+'" style="display:none;"></ul></li>';
		        gNodeIdCount++;
			}
			
			// list file items 
			for (var i=0; i<json.list_data.length; i++) {
			    if (json.list_data[i].type != 'file') {
			        continue;
			    }
			    
			    var newItem = json.list_data[i];
			    newItem.id =  'node_' + gNodeIdCount;
			    FileFolderTree.FolderList.push(newItem);
		        tmpStr += '<li><span id="node_'+gNodeIdCount+'" class="'+newItem.type+'" onclick="UpdateFocusFile(this.id)">'+newItem.name+'</span></li>';
		        gNodeIdCount++;
			}
			
			var branches = $(tmpStr).appendTo("#root_"+json.id);
	        $("#root_"+json.id).treeview({
    			add: branches
    		});	
    		
    		if (objTree.bAutoSelect) {
		           objTree.selectFolder(objTree.argDefaultFolders);
		    }
		}
	}
}

function GetShareShortPath(shareMap, fullpath)
{
    var strRet = fullpath;
    
    for (var i=0; i<shareMap.length; i++) {
	    if (fullpath.indexOf(shareMap[i].fullpath) == 0) {
	        strRet = fullpath.replace(shareMap[i].fullpath, shareMap[i].shortpath + '/');
	        break;
	    }
	}
	
    return strRet;
}



