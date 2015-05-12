function _is_select_default_bridge(button)
{
    var chked = $('#myModalnewstgForm input:radio[name="eth_pool"]').filter(':checked');
    if (chked.length == 0) {
		$('#error_msg').show();
		return false;
	}
	
	if((noSpeed[chked.val()])&&(button!='smb')){
		alert(interpolate(gettext('Please connect the network cable between %(noSpeed)s and Switch.'),{'noSpeed':noSpeed[chked.val()]},true), gotoNext);
		return false;
	}
	
	/*{% for bri_name, bri_nic, show_name in bridge_inf_ary %}
	var bri_idx = parseInt('{{ bri_name }}'.replace('br', '')) + 1;
	if ((button!='smb') && (chked.val() == '{{ bri_nic }}')) {
	    jConfirm('Remove the interface from {{ show_name }}', 'Warnning', function(result) {
    		if (result) {
    		    $('#error_msg').hide();
    		    btn_wizard(1);
    		}
		});
	    return false;
    }
    {% endfor %}*/

	$('#error_msg').hide();
    return true;
}