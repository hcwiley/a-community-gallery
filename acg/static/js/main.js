jQuery.event.add(window, 'resize', resize);
jQuery.event.add(window, 'load', init);
jQuery.event.add(window, 'unload', leave);
function printf(str){
	if(window.location.host !== 'blu-wired:8000'){
		return;
	}
	else{
		console.log(str);
	}
}
//Current page = loc
var loc;
var isMobile = false;
var isIE = false;
var fadeDelay = 400;
var changing = false;
var invalidEmail = "";
var required_email = "";
var required_message = "";
var required_name = "";
var goingOut = false;
var height;
//Checking for mobile browser
if (navigator.userAgent.match(/Android/i) ||
navigator.userAgent.match(/webOS/i) ||
navigator.userAgent.match(/iPhone/i) ||
navigator.userAgent.match(/iPod/i)) {
    isMobile = true;
}

if (jQuery.browser.msie) {
    isIE = true;
}

function sexyScroll(to, time){
    if (!to) {
        to = 0;
    }
    if (!time) {
        time = 10;
    }
    var pos = $(document).scrollTop();
    var delta = 25;
    if (pos < to - 50 || pos > to + 50) {
		if (pos > to) {
			delta *= -1;
			if (pos + delta <= to) 
				delta = pos - to;
		}
		else if (pos < to) {
            if (pos + delta >= to) 
                delta = to - pos;
        }
        window.setTimeout(function(){
            $(document).scrollTop(pos + delta);
            window.setTimeout(function(){
				sexyScroll(to);
			}, time);
        }, time * 1.5);
    }
}

function animateOut(){
    $('#container').addClass('noborders');
    $('#container').data('left-width', $('div.bottom > div.left').width());
    $('div.bottom > div.left').add($('div.bottom-copy')).animate({
        width: 0,
    }, fadeDelay).hide(0);
    $('#wrapper').data('height', $('#wrapper').height());
    window.setTimeout(function(){
        $('#wrapper').stop().animate({
            top: -2000
        }, fadeDelay * 2);
    }, fadeDelay);
}

function leave(event){
    event.preventDefault();
    window.setTimeout(function(){
        animateOut();
    }, 10);
}

function animateIn(loc){
    sexyScroll();
    window.setTimeout("$('#container').removeClass('noborders');", fadeDelay);
    $('.current-page').removeClass('current-page');
    if (loc == '/') {
        var id = '#home';
    }
    else {
        var id = '#' + loc.replace('/', '');
    }
    $(id).children().addClass('current-page');
    $.ajax({
        url: '/get' + loc,
        type: 'get',
        success: function(data){
            var title = $($(data)[0]).text();
            if (!isIE) {
                $('title').text(title);
            }
            var html = '';
            var id = '';
            data = $(data);
            for (var i = 0; i < $(data).length; i++) {
                id = $(data[i]).attr('id');
                if (id == 'content') {
                    html += $(data[i]).html();
                }
            }
            //			printf(html);
            $('#main').html(html);
            $(html).ready(function(){
                resize();
            });
            window.setTimeout(function(){
                $('#wrapper').stop().animate({
                    top: 0,
                }, fadeDelay);
            }, fadeDelay * 2);
            reinit();
            $('div.bottom > div.left').width($('#container').data('left-width'));
        },
        error: function(xhr, statusText, errorThrown){
            var html = xhr.response;
            html = $(html).find('.center');
            $('#main').html(html);
            $('#wrapper').stop().animate({
                top: 0,
            }, fadeDelay * 2);
            $('div.bottom > div.left').width($('#container').data('left-width'));
            reinit();
        }
    });
}

function watchURLChange(){
    var tmploc = window.location + "";
    tmploc = tmploc.split('/');
    tmploc = tmploc[tmploc.length - 1];
    if (changing) {
        window.setTimeout("watchURLChange();", fadeDelay * 5);
    }
    else {
        if (loc !== tmploc) {
            animateOut();
            window.setTimeout(function(){
                animateIn('/' + tmploc);
            }, fadeDelay);
        }
        else {
            window.setTimeout("watchURLChange();", 80);
        }
    }
}

function resize(){
    height = $(window).height() - $('footer').height() - $('#header').height();
    var width = $(window).width();
    var margin = (height - $('div.top').height()) / 2;
	if (isMobile) {
		margin = 20;
	}
    if (margin > 0) {
        $('div.top').css('margin-top', margin);
    }
    if ($('#big-img > img').height() > height) {
        $('#big-img > img').height(height);
        $('#big-img > img').width('auto');
    }
	if(isMobile){
		$('footer').css('top',$(document).height() - 100);
	}
}

function reinit(){
	if(isMobile){
		$('div').addClass('mobile');
	}
    //Current page = loc
    loc = window.location + "";
    loc = loc.split('/');
    loc = loc[loc.length - 1];
    if (loc == '') {
        var id = '#home';
    }
    else {
        var id = '#' + loc;
    }
    $(id).addClass('current-page');
    $('a:not(.no-link)').unbind('click');
    $('a:not(.no-link)').bind('click', function(event){
        changing = true;
        window.setTimeout("changing = false;", fadeDelay * 3);
        leave(event);
        nextPage = $(this).attr('href');
        if (nextPage == '') {
            nextPage = 'home';
        }
        if (Modernizr.history) {
            var stateObject = nextPage;
            window.history.pushState(stateObject, "", nextPage);
            if ($(this).attr('class') == 'bottom-link') {
                smoothScroll();
            }
        }
        else {
            window.location.href = nextPage;
        }
        window.setTimeout(function(){
            animateIn(nextPage);
            window.setTimeout("resize();", fadeDelay);
        }, fadeDelay * 3.5);
    });
	initBigImg();
    resize();
	if (loc !== '') {
        $('#header').add($('#pop-out')).mouseover(function(){
            popout();
        });
        $('#header').add($('#pop-out')).mouseleave(function(){
            popin();
        });
        window.setTimeout("popin();",500);
    }
	if(loc == 'gallery'){
		$(document).keydown(function(event){
            closeBig();
		});
	}
    $('#yes').bind('click', function(){
        $('div.top').css('position', 'relative');
        $('div.top').animate({
            top: -900,
            height: 0
        }, 300);
        window.setTimeout("$('div.top').remove()", 300);
        $('div.upload').css('display', 'block');
    });
	var input = $('form > *:not(p,br)').not('#submit');
	for (var i = 1; i < $(input).length; i++) {
		$(input[i]).bind('focus', function(){
			sexyScroll($(this).position().top - 200, 20);
		});
	}
    $('#submit').bind('click', function(){
        for (var i = 1; i < $(input).length; i++) {
            if ($(input[i]).val() == '' || $(input[i]).val().search(/[ ]+/g) == 0) {
                $(input[i]).focus();
                $(input[i]).addClass('missing');
                break;
            }
            else {
                $(input[i]).removeClass('missing');
            }
        }
        if ($('.missing').length == 0) {
            $('#work-form').submit();
        }
    });
    window.setTimeout("resize();", fadeDelay * 2);
}

function openBig(){
	var h = height + $('footer').height() - 5;
	sexyScroll(0 , 2);
    $('#big-img').animate({
        height: h
    }, fadeDelay);
	$('#big-img > img').height(h - $('#big-img > div.foot').height() - 35);
	$('div.top').addClass('hidden');
    $('footer').addClass('hidden');
	resize();
}

function closeBig(){
    $('#big-img').animate({
        height: 0
    }, fadeDelay);
	$('div.top').removeClass('hidden');
    $('footer').removeClass('hidden');
}

function initBigImg(){
    var imgs = $("div.img > img");
    for (var i = 0; i < $(imgs).length; i++) {
		$(imgs[i]).unbind('click');
        $(imgs[i]).bind('click', function(){
            var oldSrc = $(this).attr('src');
			$('#big-img > img').attr('src', oldSrc);
			openBig();
            var newSrc = oldSrc.replace('thumb_', '');
            $('#big-img > div.foot').html($(this).parent('div').siblings('div').html());
            if (!isIE) {
                $.get(newSrc, function(data){
                    $('#big-img > img').attr('src', newSrc);
                    $('#big-img > img').animate({
                        opacity: 1
                    }, fadeDelay);
                    if ($('#big-img > img').height() > height) {
                        $('#big-img > img').height(height - $('div.foot').height() + 35);
                        $('#big-img > img').width('auto');
                    }
                });
            }
            else {
                $('#big-img > img').attr('src', newSrc);
                $('#big-img > img').animate({
                    opacity: 1
                }, fadeDelay);
                if ($('#big-img > img').height() > height) {
                    $('#big-img > img').height(height);
                    $('#big-img > img').width('auto');
                }
            }
        });
    }
    $('#big-img').click(function(){
        closeBig();
    });
}

function popout(){
	var h = 140;
	if(isMobile)
	   h = 160;
    $('#pop-out').stop().animate({
        top: 0,
		height: h
    }, 50);
}

function popin(){
	var h = 125;
    if(isMobile)
       h = 140;
    $('#pop-out').stop().animate({
        top: -15,
		height: h
    }, 50);
}

function init(){
    resize();
    reinit();
    watchURLChange();
    window.setTimeout("resize();", 10);
}
