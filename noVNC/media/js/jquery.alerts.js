// jQuery Alert Dialogs Plugin
//
// Version 1.0
// Download by http://www.bvbsoft.com 
// Cory S.N. LaViska
// A Beautiful Site (http://abeautifulsite.net/)
// 29 December 2008
//
// Visit http://abeautifulsite.net/notebook/87 for more information
//
// Usage:
//		jAlert( message, [title, callback] )
//		jConfirm( message, [title, callback] )
//		jPrompt( message, [value, title, callback] )
// 
// History:
//
//		1.00 - Released (29 December 2008)
//
// License:
// 
//		This plugin is licensed under the GNU General Public License: http://www.gnu.org/licenses/gpl.html
//
(function($) {
	
	$.alerts = {
		
		// These properties can be read/written by accessing $.alerts.propertyName from your scripts at any time
		
		verticalOffset: -75,                // vertical offset of the dialog from center screen, in pixels
		horizontalOffset: 0,                // horizontal offset of the dialog from center screen, in pixels/
		repositionOnResize: true,           // re-centers the dialog on window resize
		overlayOpacity: .01,                // transparency level of overlay
		overlayColor: '#FFF',               // base color of overlay
		draggable: true,                    // make the dialogs draggable (requires UI Draggables plugin)
		okButton: gettext(' OK '),         // text for the OK button
		cancelButton: gettext(' Cancel '), // text for the Cancel button
		yesButton: gettext(' Yes '),         // text for the Yes button
		noButton: gettext(' No '),         // text for the No button
		dialogClass: null,                  // if specified, this class will be applied to all dialogs
		
		// Public methods
		
		alert: function(message, title, callback, blackMask) {
			if( title == null ) title = 'Alert';
			$.alerts._show(title, message, null, 'alert', function(result) {
				if( callback ) callback(result);
			}, blackMask);
		},
		
		confirm: function(message, title, callback, blackMask, OKBtn, cancelBtn) {
			if( title == null ) title = 'Confirm';
			if(blackMask){
				$.alerts.overlayOpacity=0.5;
				$.alerts.overlayColor='#000';			
			}
			
			$.alerts._show(title, message, null, 'confirm', function(result) {
				if( callback ) callback(result);
			},blackMask,OKBtn,cancelBtn);
		},
		
		confirm3: function(message, title, callback) {
			if( title == null ) title = 'Confirm';
			$.alerts._show(title, message, null, 'confirm3', function(result) {
				if( callback ) callback(result);
			});
		},
		
		prompt: function(message, value, title, callback) {
			if( title == null ) title = 'Prompt';
			$.alerts._show(title, message, value, 'prompt', function(result) {
				if( callback ) callback(result);
			});
		},
		
		// Private methods
		
		_show: function(title, msg, value, type, callback, blackMask, OKBtn, cancelBtn) {
			
			$.alerts._hide();
			$.alerts._overlay('show');
			
			var okButton = OKBtn ? OKBtn:gettext(' OK ');
			var cancelButton = cancelBtn ? cancelBtn:gettext(' Cancel ');
			
			$("BODY").append(
			  '<div id="popup_container">' +
			    '<h1 id="popup_title"></h1>' +
			    '<div id="popup_content">' +
			      '<div id="popup_message"></div>' +
				'</div>' +
			  '</div>');
			
			if( $.alerts.dialogClass ) $("#popup_container").addClass($.alerts.dialogClass);
			
			// IE6 Fix
			var pos = ($.browser.msie && parseInt($.browser.version) <= 6 ) ? 'absolute' : 'fixed'; 
			
			$("#popup_container").css({
				position: pos,
				zIndex: 99999,
				padding: 0,
				margin: 0
			});
			
			if(blackMask=='blackMaskJava')
				$("#popup_container").css('border-radius','0px');
			
			$("#popup_title").text(title);
			$("#popup_content").addClass(type);
			$("#popup_message").text(msg);
			$("#popup_message").html( $("#popup_message").text().replace(/\n/g, '<br />') );
			
			$("#popup_container").css({
				minWidth: $("#popup_container").outerWidth(),
				maxWidth: $("#popup_container").outerWidth()
			});
			
			$.alerts._reposition();
			$.alerts._maintainPosition(true);
			
			switch( type ) {
				case 'alert':
					$("#popup_content").after('<div id="popup_panel"><input type="button" class="btn" value="' + okButton + '" id="popup_ok" /></div>');
					$("#popup_ok").click( function() {
						$.alerts._hide();
						callback(true);
					});
					$("#popup_ok").focus().keypress( function(e) {
						if( e.keyCode == 13 || e.keyCode == 27 ) $("#popup_ok").trigger('click');
					});
				break;
				case 'confirm':
					$("#popup_content").after('<div id="popup_panel"><input type="button" class="btn" value="' + okButton + '" id="popup_ok" /> <input type="button" class="btn" value="' + cancelButton + '" id="popup_cancel" /></div>');
					$("#popup_ok").click( function() {
						$.alerts._hide();
						if( callback ) callback(true);
					});
					$("#popup_cancel").click( function() {
						$.alerts._hide();
						if( callback ) callback(false);
					});
					$("#popup_ok").focus();
					$("#popup_ok, #popup_cancel").keypress( function(e) {
						if( e.keyCode == 13 ) $("#popup_ok").trigger('click');
						if( e.keyCode == 27 ) $("#popup_cancel").trigger('click');
					});
				break;
				case 'confirm3':
					$("#popup_content").after('<div id="popup_panel"><input type="button" class="btn" value="' + $.alerts.yesButton + '" id="popup_yes" /> <input type="button" class="btn" value="' + $.alerts.noButton + '" id="popup_no" /> <input type="button" class="btn" value="' + cancelButton + '" id="popup_cancel" /></div>');
					$("#popup_yes").click( function() {
						$.alerts._hide();
						if( callback ) callback('yes');
					});
					$("#popup_no").click( function() {
						$.alerts._hide();
						if( callback ) callback('no');
					});
					$("#popup_cancel").click( function() {
						$.alerts._hide();
						if( callback ) callback('cancel');
					});
					$("#popup_yes").focus();
					$("#popup_yes, #popup_no, #popup_cancel").keypress( function(e) {
						if(( e.keyCode == 13 ) || ( String.fromCharCode(e.charCode) == 'y' )) $("#popup_yes").trigger('click'); // enter, y
						if( String.fromCharCode(e.charCode) == 'n' ) $("#popup_no").trigger('click'); // n
						if( e.keyCode == 27 ) $("#popup_cancel").trigger('click'); // escape
					});
				break;
				case 'prompt':
					$("#popup_content").append('<br /><input type="text" size="30" id="popup_prompt" />').after('<div id="popup_panel"><input type="button" class="btn" value="' + okButton + '" id="popup_ok" /> <input type="button" class="btn" value="' + cancelButton + '" id="popup_cancel" /></div>');
					$("#popup_prompt").width( $("#popup_message").width() );
					$("#popup_ok").click( function() {
						var val = $("#popup_prompt").val();
						$.alerts._hide();
						if( callback ) callback( val );
					});
					$("#popup_cancel").click( function() {
						$.alerts._hide();
						if( callback ) callback( null );
					});
					$("#popup_prompt, #popup_ok, #popup_cancel").keypress( function(e) {
						if( e.keyCode == 13 ) $("#popup_ok").trigger('click');
						if( e.keyCode == 27 ) $("#popup_cancel").trigger('click');
					});
					if( value ) $("#popup_prompt").val(value);
					$("#popup_prompt").focus().select();
				break;
			}
			
			// Make draggable
			if( $.alerts.draggable ) {
				try {
					$("#popup_container").draggable({ handle: $("#popup_title") });
					$("#popup_title").css({ cursor: 'move' });
				} catch(e) { /* requires jQuery UI draggables */ }
			}
		},
		
		_hide: function() {
			$("#popup_container").remove();
			$.alerts._overlay('hide');
			$.alerts._maintainPosition(false);
		},
		
		_overlay: function(status) {
			switch( status ) {
				case 'show':
					$.alerts._overlay('hide');
					$("BODY").append('<div id="popup_overlay"></div>');
					$("#popup_overlay").css({
						position: 'absolute',
						zIndex: 99998,
						top: '0px',
						left: '0px',
						width: '100%',
						height: $(document).height(),
						background: $.alerts.overlayColor,
						opacity: $.alerts.overlayOpacity
					});
					
					$("#popup_overlay").append('<IFRAME style="Z-INDEX: -1; POSITION: absolute; WIDTH: 100%; HEIGHT: 100%; TOP: 0px; LEFT: 0px" frameBorder=0></IFRAME>');
				break;
				case 'hide':
					$("#popup_overlay").remove();
				break;
			}
		},
		
		_reposition: function() {
			var top = (($(window).height() / 2) - ($("#popup_container").outerHeight() / 2)) + $.alerts.verticalOffset;
			var left = (($(window).width() / 2) - ($("#popup_container").outerWidth() / 2)) + $.alerts.horizontalOffset;
			if( top < 0 ) top = 0;
			if( left < 0 ) left = 0;
			
			// IE6 fix
			if( $.browser.msie && parseInt($.browser.version) <= 6 ) top = top + $(window).scrollTop();
			
			$("#popup_container").css({
				top: top + 'px',
				left: left + 'px'
			});
			$("#popup_overlay").height( $(document).height() );
		},
		
		_maintainPosition: function(status) {
			if( $.alerts.repositionOnResize ) {
				switch(status) {
					case true:
						$(window).bind('resize', function() {
							$.alerts._reposition();
						});
					break;
					case false:
						$(window).unbind('resize');
					break;
				}
			}
		}
		
	}
	
	// Shortuct functions
	jAlert = function(message, title, callback, blackMask) {
		$.alerts.alert(message, title, callback, blackMask);
	}
	
	jConfirm = function(message, title, callback, blackMask, OKBtn, cancelBtn) {
		$.alerts.confirm(message, title, callback, blackMask, OKBtn, cancelBtn);
	};
	
	jConfirm3 = function(message, title, callback) {
		$.alerts.confirm3(message, title, callback);
	};
		
	jPrompt = function(message, value, title, callback) {
		$.alerts.prompt(message, value, title, callback);
	};
	
})(jQuery);
