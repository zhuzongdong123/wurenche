
console.log("●●●●●● hik index.js");
(function () {
	// 检测浏览器版本
    var explorer = window.navigator.userAgent;
    var language = window.navigator.systemLanguage || window.navigator.language;
	language = language.slice(0, 2);
	console.log(explorer);
    if (explorer.indexOf("MSIE") >= 0) {
		var browser = navigator.appName;
		var b_version = navigator.appVersion;
		var version = b_version.split(";");
		var trim_Version = version[1].replace(/[ ]/g, "");
		var number ="";
		for (i = 0; i < trim_Version.length; i++) {
			if ("0123456789".indexOf(trim_Version.substr(i, 1)) > -1)
				number += trim_Version.substr(i, 1);
		}
		console.log(number);
		if (number < 110){
			if(language == "zh"){
				var text = "当前浏览器版本过低，无法正常显示。请升级Internet Explorer至版本 11及以上。";
				alert(text);
			} else {
				var text = "Display exception. The current browser version is too low. Please upgrade the Internet Explorer to version 11 or above.";
				alert(text);
			}
		}
    }
})();

$(document).ready(function () {
	var guid = getParameter('guid');
	var hl = getParameter('hl');
	
	if($(document).width() < 510){
		console.log("●●●●●● Mobile: 显示 container_mobile");
		/* 显示 container_mobile*/
		$('.container_mobile').show();
		
		
		/*wh_top_menu_and_indexterms_link的最大高度为页面的最大高度 的一半 */
		$('.wh_top_menu_and_indexterms_link').eq(0).css('max-height', $(window).height() * 0.4 + 'px');
		/*修改默认的手机端右上角目录按钮事件*/
		bindClick_Mobile();
		
		//搜索页面跳转
		if (guid !='' && guid !== undefined && hl !='' && hl != 'undefined') {
			var frame_src = guid + "?hl=" + decodeURIComponent(hl);
			$("#frame_mobile").attr("src", frame_src);
			$("#mobile_menu").css("display", "none");
		}
	} else {
		// desktop 显示目录的全部
		console.log("●●●●●● Desktop: 显示目录的全部 desktop_left ");
		setDesktopMenuMove();
		console.log("●●●●●● Desktop 显示 container_desktop");
		/* 显示 container_desktop*/
		$('.container_desktop').show();
		
		if (guid !='' && guid !== undefined && hl !='' && hl != 'undefined') {
			//搜索页面跳转
			var frame_src = guid + "?hl=" + decodeURIComponent(hl);
			$("#frame_desktop").attr("src", frame_src);
			//$(".menu_ul li").removeClass('active');
			//高亮左侧菜单 desktop_left menu_ul
			$('.desktop_left li').each(function(){
				var a = $(this).find("a").eq(0);
				if(a.attr('data-href') != null && a.attr('data-href') == guid){
					//激活
					active_li_ancestor($(this));
					//判断是否需要滚动
					var scrollTopMenu = $(this).parents('.desktop_left_menu_ul').eq(0).scrollTop();
					var liOffsetTop = $(this).parent().offset().top;
					var menuHeight = $(this).parents('.desktop_left_menu_ul').eq(0).height();
					if (menuHeight < liOffsetTop) {
						$(this).parents('.desktop_left_menu_ul').eq(0).scrollTop(scrollTopMenu + liOffsetTop - 60)
					}
				}
			});
		} else {
			// index.html页面跳转到第一个topic
			console.log("●●●●●● Desktop index.html首页跳转");
			var a = $('.desktop_left_menu_ul').eq(0).children('.menu_ul').eq(0).children('li').eq(0).find('a').eq(0);
			if(a.attr('data-href') != null && a.attr('data-href') != ''){
				$("#frame_desktop").attr("src", a.attr('data-href'));
				//激活状态
				$('.desktop_left_menu_ul').eq(0).children('.menu_ul').eq(0).children('li').eq(0).addClass('active');
			}
			
		}
	}
	// 好像没有什么区别
	var container_mobile = $('.container_mobile');
    if($(document).width() < 510){
		container_mobile.css('flex-direction', 'column');
    } else {
		container_mobile.css('flex-direction', 'initial');
	}
	
});

/* 激活li, 打开上层菜单 */
function active_li_ancestor(li_obj){
	li_obj.addClass('active');
	var parent_li = li_obj.parent().parent();
	if(parent_li[0].nodeName == "LI"){
		parent_li.removeClass('li_close');
		parent_li.addClass('li_open');
		active_li_ancestor(parent_li);
	}
}

/*记录li_has_children的li_open和li_close*/
var liStatus;
if($.cookie('li_status') != null){
	console.log("li_status");
	liStatus = str2array($.cookie('li_status'));
	if(liStatus.length == $('.li_has_children').length){
		for(var i = 0; i < $('.li_has_children').length; i++){
			if(liStatus[i] == 'li_open'){
				setLiStatusOpen($('.li_has_children').eq(i));
			}else if(liStatus[i] == 'li_close'){
				setLiStatusClose($('.li_has_children').eq(i));
			}
		}
	}else{
		liStatus = new Array();
		$('.li_has_children').each(function(){
			liStatus.push('li_close');
			setLiStatusClose($(this));
		});
		$.cookie('li_status', array2str(liStatus), { expires: 7 });
	}
}else{
	liStatus = new Array();
	$('.li_has_children').each(function(){
		liStatus.push('li_close');
		setLiStatusClose($(this));
	});
	$.cookie('li_status', array2str(liStatus), { expires: 7 });
}

/*增加li_before的点击事件*/
$('.li_before').unbind('click').bind('click', function(){
	console.log("●● li_before click ");
	var liPosition = $('.li_has_children').index($(this).parent().parent());
	if($(this).parent().parent().hasClass('li_close')){
		$(this).parent().parent().removeClass('li_close');
		$(this).parent().parent().addClass('li_open');
		liStatus[liPosition] = 'li_open';
	}else if($(this).parent().parent().hasClass('li_open')){
		$(this).parent().parent().addClass('li_close');
		$(this).parent().parent().removeClass('li_open');
		liStatus[liPosition] = 'li_close';
	}
	//$.cookie('li_status', array2str(liStatus), { expires: 7 });
}); 

function setLiStatusOpen(obj){
	obj.removeClass('li_close');
	obj.addClass('li_open');
}
function setLiStatusClose(obj){
	obj.removeClass('li_open');
	obj.addClass('li_close');
}
function array2str(array){
	var str = "";
	for(var i = 0; i < array.length; i++){
		if(i != array.length - 1){
			str = str + array[i] + "-";
		}else{
			str = str + array[i];
		}
	}
	return str;
}
function str2array(str){
	var array = new Array();
	for(var i = 0; i < str.split('-').length; i++){
		array.push(str.split('-')[i]);
	}
	return array;
}



/*当页面滚动时，取消输入框自动补齐的Autocomplete功能*/
$(document).scroll(function(){
	$("#textToSearch").autocomplete("close");
});
/*监听窗口的滚动事件*/
var startX, startY;  
document.addEventListener('touchstart', function(ev){
    startX = ev.touches[0].pageX;
    startY = ev.touches[0].pageY;
}, false);
document.addEventListener('touchend',function (ev) {  
    var endX, endY;  
    endX = ev.changedTouches[0].pageX;  
    endY = ev.changedTouches[0].pageY;  
    var direction = GetSlideDirection(startX, startY, endX, endY);
    switch(direction){
        case 0:
            break;
        case 1:
            // 手指向上滑动
            //$('.hikwh_header').css('display', 'none');
            //$('.wh_main_page_search').css('display', 'none');
            //$('.hikwh_search_input').css('display', 'none');
            var scrollTop = 0;
            var scrollTimer = setInterval(function(){
				if(scrollTop != $(document).scrollTop()){
					scrollTop = $(document).scrollTop();
				}else{
					scrollTop = $(document).scrollTop();
					clearInterval(scrollTimer);
					//alert(scrollTop);
					if($(document).scrollTop() > 60){
		            	$('.hikwh_header').css('display', 'none');
		            	$('.wh_main_page_search').css('display', 'none');
		            	$('.hikwh_search_input').css('display', 'none');
		            }
				}
			}, 50);
            break;
        case 2:
            // 手指向下滑动
            //alert(endScrollTop());
            var scrollTop = 0;
            var scrollTimer = setInterval(function(){
				if(scrollTop != $(document).scrollTop()){
					scrollTop = $(document).scrollTop();
				}else{
					scrollTop = $(document).scrollTop();
					clearInterval(scrollTimer);
					//alert(scrollTop);
					if($(document).scrollTop() <= 60){
		            	$('.hikwh_header').css('display', 'block');
		            	$('.wh_main_page_search').css('display', 'block');
		            	$('.hikwh_search_input').css('display', 'block');
		            }
				}
			}, 50);
            break;
    }
}, false);

function GetSlideDirection(startX, startY, endX, endY) {  
    var dy = startY - endY;  
    var dx = endX - startX;  
    var result = 0; 
    if(dy > 0){//向上滑动
        return 1;
    }else{//向下滑动
        return 2;
    }
    if(dx > 0){//向右滑动
    	return 3;
    }else{//向左滑动
    	return 4;
    } 
}

function setDesktopMenuMove(){
    $('.desktop_left').find('a').unbind('mouseover').bind('mouseover', function(){
        var flag = isEllipsis($(this)[0]);
        if(flag){
            $('.menu-tempdiv').html($(this).html());
            $('.menu-tempdiv').removeClass('hidden');
            $('.menu-tempdiv').addClass('show');
            $('.menu-tempdiv').css('top', $(this).offset().top + 29 - $('body,html').scrollTop() + 'px');
            $('.menu-tempdiv').css('left', $(this).offset().left + 'px');
            /*if($(this).parent().hasClass('li-1')){
                $('.menu-tempdiv').css('left', $(this).offset().left + 36 + 'px');
            }else if($(this).parent().hasClass('li-2')){
                $('.menu-tempdiv').css('left', $(this).offset().left + 72 + 'px');
            }else if($(this).parent().hasClass('li-3')){
                $('.menu-tempdiv').css('left', $(this).offset().left + 108 + 'px');
            }*/
        }
    })
    $('.desktop_left').find('a').unbind('mouseout').bind('mouseout', function(){
        $('.menu-tempdiv').removeClass('show');
        $('.menu-tempdiv').addClass('hidden');
    })
}
function isEllipsis(dom) {
    var checkDom = dom.cloneNode(), parent, flag;
    checkDom.style.width = dom.offsetWidth + 'px';
    checkDom.style.height = dom.offsetHeight + 'px';
    checkDom.style.overflow = 'auto';
    checkDom.style.position = 'absolute';
    checkDom.style.zIndex = -1;
    checkDom.style.opacity = 0;
    checkDom.style.whiteSpace = "nowrap";
    checkDom.innerHTML = dom.innerHTML;
 
    parent = dom.parentNode;
    parent.appendChild(checkDom);
    flag = checkDom.scrollWidth > checkDom.offsetWidth;
    parent.removeChild(checkDom);
    return flag;
}


// Mobile
function bindClick_Mobile(){
	$('.wh_toggle_button').bind('click', function(event){
		console.log("●● wh_toggle_button click ");
		if($('.wh_top_menu_and_indexterms_link').hasClass('in')){
			//收缩
			$('.wh_toggle_button').css('background-color', '#2d2d2d');
			$('.wh_toggle_button').find('.icon-bar').css('background-color', 'white');
			//高度重新计算  减去 $('.wh_top_menu_and_indexterms_link') 高度
			var top_menu_height = $('.wh_top_menu_and_indexterms_link').eq(0).height();
			var container_height = $(window).height() - top_menu_height - 170;
			$(".wh_content_mobile_container").height("calc(100% - 45px)");
			
		
		}else{
			//展开
			$('.wh_toggle_button').css('background-color', '#ddd');
			$('.wh_toggle_button').find('.icon-bar').css('background-color', '#333');
			//高度重新计算  减去 $('.wh_top_menu_and_indexterms_link') 高度
			setTimeout(function(){
				var top_menu_height = $('.wh_top_menu_and_indexterms_link').eq(0).height();
				var container_height = $(window).height() - top_menu_height - 180;
				$(".wh_content_mobile_container").height(container_height + "px");
				
			},500); //500毫秒后执行
			
			
		}
	});
	// 展开后，点击其他地方，收缩导航栏
	$('#content_area').bind('click', function(){
		if($('.wh_top_menu_and_indexterms_link').hasClass('in')){
			$('.wh_toggle_button').click();
		}
	});
	$('.hikwh_header').bind('click', function(){
		if($('.wh_top_menu_and_indexterms_link').hasClass('in')){
			$('.wh_toggle_button').click();
		}
	});
	$('.hikwh_search_input').bind('click', function(){
		if($('.wh_top_menu_and_indexterms_link').hasClass('in')){
			$('.wh_toggle_button').click();
		}
	});
    
}

//获取url参数
function getQueryString(name) {
    var result = window.location.search.match(new RegExp("[\?\&]" + name + "=([^\&]+)", "i"));
    if (result == null || result.length < 1) {
        return "";
    }
    var gn = result[1];
    if (!(gn.indexOf("%") < 0)) {
        try {
            gn = unescape(gn)
        } catch (e) {
            gn = decodeURI(gn);

        }
    }
    return gn;
}


function expandMenu(){
	$('.desktop_left').find('li').each(function(){
        //扩张
		$(this).removeClass('li_close');
		$(this).addClass('li_open');
    })
}
function collapseMenu(){
	$('.desktop_left').find('li').each(function(){
        //折叠
		$(this).removeClass('li_open');
		$(this).addClass('li_close');
    })
}

addEventListener('message', e => {
	var event_href = e.data;	
	// 接收 iframe 内 鼠标点击事件，收缩 mobile 顶部菜单。
	console.log("●● receive postMessage");
	if($('.wh_top_menu_and_indexterms_link').hasClass('in')){
		$('.wh_toggle_button').click();
	} else {
		//高亮左侧菜单 desktop_left menu_ul
		console.log("高亮左侧菜单:" + event_href);
		gotoActiveToc(event_href);
	}
	
	
	if (event && event.stopPropagation){
		event.stopPropagation();
	} else {
		window.event.cancelBubble = true;
	}
})

function gotoActiveToc(guid_html){
	//先取消激活
	$('.active').each(function(){
		$(this).removeClass('active');
	});
	//
	$('.desktop_left li').each(function(){
		var a = $(this).find("a").eq(0);
		if(a.attr('data-href') != null && a.attr('data-href') == guid_html){
			//激活
			active_li_ancestor($(this));
			//判断是否需要滚动
			var scrollTopMenu = $(this).parents('.desktop_left_menu_ul').eq(0).scrollTop();
			var liOffsetTop = $(this).parent().offset().top;
			var menuHeight = $(this).parents('.desktop_left_menu_ul').eq(0).height();
			if (menuHeight < liOffsetTop) {
				$(this).parents('.desktop_left_menu_ul').eq(0).scrollTop(scrollTopMenu + liOffsetTop - 60)
			}
		}
	});
}
