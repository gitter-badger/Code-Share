$(function(){
    $('.dropmenu').hover(function(){
        $('.navbar-footer').addClass('dropped');
    }, function(){
        $('.navbar-footer').removeClass('dropped');
    });
});