/*
  Bootstrap - File Input
  ======================

  This is meant to convert all file input tags into a set of elements that displays consistently in all browsers.

  Converts all
  <input type="file">
  into Bootstrap buttons
  <a class="btn">Browse</a>

*/
$(function() {

$('input[type=file]').each(function(i,elem){

  // Maybe some fields don't need to be standardized.
  if (typeof $(this).attr('data-bfi-disabled') != 'undefined') {
    return;
  }

  // Set the word to be displayed on the button
  var buttonWord = gettext('Browse');

  if (typeof $(this).attr('title') != 'undefined') {
    buttonWord = $(this).attr('title');
  }

  // Start by getting the HTML of the input element.
  // Thanks for the tip http://stackoverflow.com/a/1299069
  var $elem = $(elem);
  var input = $('<div>').append( $elem.eq(0).clone() ).html();

  // Now we're going to replace that input field with a Bootstrap button.
  // The input will actually still be there, it will just be float above and transparent (done with the CSS).
  $elem.replaceWith('<a class="file-input-wrapper btn ' + $elem.attr('class') + '">'+buttonWord+input+'</a>');
})
// After we have found all of the file inputs let's apply a listener for tracking the mouse movement.
// This is important because the in order to give the illusion that this is a button in FF we actually need to move the button from the file input under the cursor. Ugh.
.promise().done( function(){

  // As the cursor moves over our new Bootstrap button we need to adjust the position of the invisible file input Browse button to be under the cursor.
  // This gives us the pointer cursor that FF denies us
  $('.file-input-wrapper').mousemove(function(cursor) {

    var input, wrapper,
      wrapperX, wrapperY,
      inputWidth, inputHeight,
      cursorX, cursorY;

    // This wrapper element (the button surround this file input)
    wrapper = $(this);
    // The invisible file input element
    input = wrapper.find("input");
    // The left-most position of the wrapper
    wrapperX = wrapper.offset().left;
    // The top-most position of the wrapper
    wrapperY = wrapper.offset().top;
    // The with of the browsers input field
    inputWidth= input.width();
    // The height of the browsers input field
    inputHeight= input.height();
    //The position of the cursor in the wrapper
    cursorX = cursor.pageX;
    cursorY = cursor.pageY;

    //The positions we are to move the invisible file input
    // The 20 at the end is an arbitrary number of pixels that we can shift the input such that cursor is not pointing at the end of the Browse button but somewhere nearer the middle
    moveInputX = cursorX - wrapperX - inputWidth + 20;
    // Slides the invisible input Browse button to be positioned middle under the cursor
    moveInputY = cursorY- wrapperY - (inputHeight/2);

    // Apply the positioning styles to actually move the invisible file input
    input.css({
      left:moveInputX,
      top:moveInputY
    });
  });

  $('.file-input-wrapper input[type=file]').change(function(){

    // Remove any previous file names
    $(this).parent().next('.file-input-name').remove();
    if ($(this).prop('files').length > 1) {
      count = $(this).prop('files').length;
      str = '<div class="file-input-name">'
      for (var i=0; i<count ; i++)
      {
        str += this.files[i].name+'<br>';
      }
      str += '</div>';
      $(this).parent().after(str);
    }
    else {
      $(this).parent().after('<div class="file-input-name">'+$(this).val().replace('C:\\fakepath\\','')+'</div>');
    }

  });

  

});

// Add the styles before the first stylesheet
// This ensures they can be easily overridden with developer styles
var cssHtml = '<style>'+
  '.file-input-wrapper { overflow: hidden; position: relative; cursor: pointer; z-index: 1; }'+
  '.file-input-wrapper input[type=file], .file-input-wrapper input[type=file]:focus, .file-input-wrapper input[type=file]:hover { position: absolute; top: 0; left: 0; cursor: pointer; opacity: 0; filter: alpha(opacity=0); z-index: 99; outline: 0; }'+
  '.file-input-name { padding:5px 5px 0px 20px; }'+
  '</style>';
$('link[rel=stylesheet]').eq(0).before(cssHtml);

});
