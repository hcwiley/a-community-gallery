jQuery.event.add(window, 'resize', resize);
jQuery.event.add(window, 'load', init);
function printf(str){
    if (window.location.host !== 'blu-wired:8000') {
        return;
    }
    else {
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
var margin;
var bottom;

function resize(){
    height = $(window).height();
    var width = $(window).width();
    margin = (height - $('div.top').height()) / 4;
    if (isMobile) {
        margin = 20;
    }
    //    if (margin > 0) {
    $('div.top').css('margin-top', margin);
    //    }
    if ($('#big-img > img').height() > height) {
        $('#big-img > img').height(height);
        $('#big-img > img').width('auto');
    }
    if (isMobile) {
        $('footer').css('top', $(document).height() - 100);
    }
}

function scrollGallery(){
    if ($('#container').position().top > (-1 * $(document).height() + 600)) {
        $('#container').stop(false, true).animate({
            top: '-=' + 5
        }, 800);
        window.setTimeout("scrollGallery();", 150);
    }
    else {
        window.setTimeout(function(){
            $('#container').stop(false, true).animate({
                top: '-=' + 1000
            }, 800);
            window.setTimeout("window.location = window.location;", 800);
        }, 8000);
    }
}


function changeText(goTo){
    if (goTo == 'middle') {
		margin += $('div.top').height() + $('div.middle').height() + 800;
        $('#container').animate({
            top: '-=' + margin
        }, 800);
        window.setTimeout("changeText('gallery');", 15000);
    }
    else if (goTo == 'gallery') {
		margin -= 200;
        $('#container').animate({
            top: '-=' + margin
        }, 800);
        window.setTimeout("scrollGallery();", 200);
    }
    else if (goTo == 'top') {
        $('#container').animate({
            top: 0
        }, 800);
        window.setTimeout("changeText('middle');", 10000);
    }
}

function init(){
    resize();
    window.setTimeout("changeText('middle');", 10000);
    window.setTimeout("resize();", 10);
}
