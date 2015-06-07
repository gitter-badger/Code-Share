$(function(){
    var drop1 = 0;
    var drop2 = 0;
    $('.dropmenu').hover(function(){
        $('.navbar-footer').addClass('dropped');
        $('.container').addClass('dropped');
    }, function(){
        $('.navbar-nav li').not('.dropmenu').hover(function(){
            $('.navbar-footer').removeClass('dropped');
            $('.container').removeClass('dropped');
        });
        $('.navbar-footer').mouseleave(function(){
            $('.navbar-footer').removeClass('dropped');
            $('.container').removeClass('dropped');
        });
    });
});
