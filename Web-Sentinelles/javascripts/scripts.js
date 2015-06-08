$(function(){
    var look = 0;
    $('.dropmenu').hover(function(){
        $('#dropper').addClass('dropped');
    },function(){
        $('.navbar-footer').hover(function(){
            look = 1;
        },function(){
            look = 0;
            $('#dropper').removeClass('dropped');
        });
        setTimeout(function(){
            if(look == 0){
                $('#dropper').removeClass('dropped');
            };
        },1000);
    });
});
