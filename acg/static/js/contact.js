jQuery.event.add(window, 'load', initContact);


function initContactForm(invalid_email1, required_email1, required_message1, required_name1){
    invalid_email = invalid_email1;
    required_email = required_email1;
    required_message = required_message1;
    required_name = required_name1;
}

function submitContactForm(){
    $('#form-feedback > h2').html('');
    if ($('#name').val() == '' || $('#name').val().search(/[ ]+/g) == 0) {
        $('#name').focus();
        $('#form-feedback > h2').html(required_name);
    }
    else if ($('#email').val() == '') {
        $('#email').focus();
        $('#form-feedback > h2').html(required_email);
    }
    else if ($('#email').val().split('@').length < 2) {
        $('#email').focus();
        $('#form-feedback > h2').html(invalid_email);
    }
    else if ($('#email').val().split('@')[1].split('.').length < 2) {
        $('#email').focus();
        $('#form-feedback > h2').html(invalid_email);
    }
    else if ($('#comments').val() == '' || $('#comments').val() == 'Message*') {
        $('#comment').focus();
        $('#form-feedback > h2').html(required_mesage);
    }
    else {
        $('#send').trigger('mouseover');
        $('#send').unbind();
        $.post('/contact', $('#contact-form-id').serialize(), function(data){
            $('#form-feedback > h2').text(data);
        });
    }
}


function initContact(){
	if (isIE) {
		if (navigator.userAgent.match('MSIE 6')) {
			$('form').remove();
		}
		$('body').add('div').addClass('ie');
		//Contact page stuff
		$('#name').val('Name*');
		$('#name').bind('focus', function(){
			if ($(this).val() == 'Name*') 
				$(this).val('');
		});
		$('#name').bind('blur', function(){
			if ($(this).val() == '') 
				$(this).val('Name*');
		});
		$('#email').val('Email*');
		$('#email').bind('focus', function(){
			if ($(this).val() == 'Email*') 
				$(this).val('');
		});
		$('#email').bind('blur', function(){
			if ($(this).val() == '') 
				$(this).val('Email*');
		});
		$('#telephone').val('Phone');
		$('#telephone').bind('focus', function(){
			if ($(this).val() == 'Phone') 
				$(this).val('');
		});
		$('#telephone').bind('blur', function(){
			if ($(this).val() == '') 
				$(this).val('Phone');
		});
		$('#comments').bind('blur', function(){
			if ($(this).val() == '') 
				$(this).val('Message*');
		});
	}
	$('#send').click(function(){
		submitContactForm();
	});
	$('input').add('#comments').blur(function(){
		if ($(this).val().search(/^[ \t]+/g) == 0) 
			$(this).val($(this).val().replace(/^[ \t]+/g, ''));
	});
	$('#contact-form-id').submit(function(){
		submitContactForm();
	});
	$('#comments').val('Message*');
	$('#comments').bind('focus', function(){
		if ($(this).val() == 'Message*') 
			$(this).val('');
	});
	var sendTrans = 250;
	$('#send').bind('mouseover', function(){
		$('#send > img:first').stop().animate({
			opacity: 0
		}, sendTrans);
		$('#send > img:last').stop().animate({
			opacity: 1
		}, sendTrans);
	});
	$('#send').bind('mouseleave', function(){
		$('#send > img:first').stop().animate({
			opacity: 1
		}, sendTrans);
		$('#send > img:last').stop().animate({
			opacity: 0
		}, sendTrans);
	});
}