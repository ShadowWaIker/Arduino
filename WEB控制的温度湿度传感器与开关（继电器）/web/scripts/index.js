$(function(){
	//alert("加载完成");
	setInterval(function(){
   		/*$.getJSON("http://10.0.0.254/?jsoncallback=?",function(result){
   		    $.each(result, function(i, field){
   		        $("div").append(field + " ");
   		    });
   		});*/
   		$.ajax({
			type : "get", //jquey是不支持post方式跨域的
			//async:false, //异步执行
			url : "http://10.0.0.254/", //跨域请求的URL
			//dataType : "jsonp",  //服务端增加响应头后不需要jsonp跨域
			//传递给请求处理程序，用以获得jsonp回调函数名的参数名(默认为:callback)
			//jsonp: "jsoncallback",
			//自定义的jsonp回调函数名称，默认为jQuery自动生成的随机函数名
			//jsonpCallback:"success_jsonpCallback",
			//成功获取跨域服务器上的json数据后,会动态执行这个callback函数
			success : function(jsonStr){ 
				var json = $.parseJSON(jsonStr);
				$(".Cabinet_Basic").html("<h1>当前NAS环境</h1><br>温度："+json.BMP180_Temperature+"℃<br>气压："+
					json.BMP180_Pressure+"hPa<br>海拔："+json.BMP180_Altitude+"M<br><br><h1>当前机柜环境</h1><br>温度："+json.DHT11_Temperature+
					"℃<br>湿度："+json.DHT11_Humidity+"%<br>露点："+json.DHT11_Dew_Point+
					"℃");
				$(".Cabinet_Switch").show();
				//设置各个按钮的颜色
				$.each(json.Switch,function(index,value){
				    //console.log(index+"..."+value);
				    if(value == "ON"){
				    	$(".button#"+index).removeClass("button-highlight").addClass('button-primary');
				    } else {
				    	$(".button#"+index).removeClass("button-primary").addClass('button-highlight');
				    }
				});
				return;
			}
		});
   	},2000);

    $(".button").click(function () {
        var val=$(this).attr("id");
        if(confirm("确定操作?")){
			//点击确定后操作
			$.post("http://10.0.0.254/",{sw:val},function(result){
	    		//$("span").html(result);
	    		if(val > 2){
	    			alert("开关"+val+"修改成功！");
	    		}
	    	});
		}
    });

    // 基于准备好的dom，初始化echarts实例
    var myChart = echarts.init(document.getElementById('main'));

        // 指定图表的配置项和数据
        // 温度/露点
        Temp_Option = {
	        title: {
	            text: 'Beijing AQI'
	        },
	        tooltip: {
	            trigger: 'axis'
	        },
	        xAxis: {
	            /*data: data.map(function (item) {
	                return item[0];
	            })*/
	        },
	        yAxis: {
	            splitLine: {
	                show: false
	            }
	        },
	        toolbox: {
	            left: 'center',
	            feature: {
	                dataZoom: {
	                    yAxisIndex: 'none'
	                },
	                restore: {},
	                saveAsImage: {}
	            }
	        },
	        dataZoom: [{
	            startValue: '2014-06-01'
	        }, {
	            type: 'inside'
	        }],
	        visualMap: {
	            top: 10,
	            right: 10,
	            pieces: [{
	                gt: 0,
	                lte: 50,
	                color: '#096'
	            }, {
	                gt: 50,
	                lte: 100,
	                color: '#ffde33'
	            }, {
	                gt: 100,
	                lte: 150,
	                color: '#ff9933'
	            }, {
	                gt: 150,
	                lte: 200,
	                color: '#cc0033'
	            }, {
	                gt: 200,
	                lte: 300,
	                color: '#660099'
	            }, {
	                gt: 300,
	                color: '#7e0023'
	            }],
	            outOfRange: {
	                color: '#999'
	            }
	        },
	        series: {
	            name: 'Beijing AQI',
	            type: 'line',
	            /*data: data.map(function (item) {
	                return item[1];
	            }),*/
	            markLine: {
	                silent: true,
	                data: [{
	                    yAxis: 50
	                }, {
	                    yAxis: 100
	                }, {
	                    yAxis: 150
	                }, {
	                    yAxis: 200
	                }, {
	                    yAxis: 300
	                }]
	            }
	        }
	    }
        // 气压
        // 湿度
        // 露点
        var option = {
            title: {
                text: 'ECharts 入门示例'
            },
            tooltip: {},
            legend: {
                data:['销量']
            },
            xAxis: {
                data: ["衬衫","羊毛衫","雪纺衫","裤子","高跟鞋","袜子"]
            },
            yAxis: {},
            series: [{
                name: '销量',
                type: 'bar',
                data: [5, 20, 36, 10, 10, 20]
            }]
        };

        // 使用刚指定的配置项和数据显示图表。
        myChart.setOption(option);
});