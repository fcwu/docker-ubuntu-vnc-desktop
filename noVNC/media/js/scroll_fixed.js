function scroll_fixed()
{
	var lastScrollLeft=0;

	$('#rightDiv').scroll(function(event) {
	    var documentScrollLeft = $(this).scrollLeft();
	    
	    if (lastScrollLeft != documentScrollLeft) {
		var newLeft=(-1*documentScrollLeft)-30;
		$('.scroll_fixed').css({'margin-left':newLeft});
		lastScrollLeft = documentScrollLeft;
	    }

	});
	
}