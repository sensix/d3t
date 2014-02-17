(function($){
	
  Drupal.behaviors.d3t = {
    attach: function(context, settings){
    	$.get(settings.basePath + 'd3t/discipline.json', function(data){
    		$.each(data, function(i,n){
    			$('<li><a id="selctor-data-tid-'+n.tid+'" href="'+n.tid+'" data-tid="'+n.tid+'">'+n.name+'</a></li>').appendTo('#discipline');
    		});
    		$('#discipline a').click(function(event){
    			event.preventDefault();
    			$('.selector a').removeClass("active");
    			$(this).addClass("active");
    			draw('M', event.target.dataset.tid);
    			content('D', event.target.dataset.tid, 0, 0);
    		});
    	});
    	$.get(settings.basePath + 'd3t/aree_tematiche.json', function(data){
    		$.each(data, function(i,n){
    			if(i == 0){
    				//carica la mappa la prima volta!!
    				draw('D', n.tid);
    				//carica i contenuti associati alla mappa tematica
    				content('M', n.tid, 0, 0);
        			$('<li><a class="active" id="selctor-data-tid-'+n.tid+'" href="'+n.tid+'" data-tid="'+n.tid+'">'+n.name+'</a></li>').appendTo('#aree-tematiche');
    			}else{
        			$('<li><a id="selctor-data-tid-'+n.tid+'" href="'+n.tid+'" data-tid="'+n.tid+'">'+n.name+'</a></li>').appendTo('#aree-tematiche');
    			}
    		});
    		$('#aree-tematiche a').click(function(event){
    			event.preventDefault();
    			$('.selector a').removeClass("active");
    			$(this).addClass("active");
    			draw('D', event.target.dataset.tid);
       			content('M', event.target.dataset.tid, 0, 0);
       		 
    		});
    	});
    	
    
    	function draw(type, tid){
    		
    	var data_path = settings.basePath + 'd3t/graph/' + type + '/' + tid;
    	
		var min = 99999;
		var max = 0;
    	$.ajax({url: data_path, async: false}).done(function(data){

    		$.each(data.nodes, function(i,n){
    			if(n.size > max){
    				max = n.size;
    			}
    			if(n.size < min){
    				min = n.size;
    			}
    		});
    	});
    	
    	var width = 840,
        height = 400;
    	
    	var linearScale = d3.scale.linear()
    		.domain([min,max])
    		.range([30, 70]);

    var color = d3.scale.category20();

    var force = d3.layout.force()
        .charge(-200)
        .gravity(0.1)
        .linkDistance(250)
        .size([width, height]);

    $(".graph").empty();
    var svg = d3.select(".graph").append("svg")
        .attr("width", width)
        .attr("height", height);

    d3.json(data_path, function(error, graph) {
    	if(error){
    		alert(error.message);
    	}
      force
          .nodes(graph.nodes)
          .links(graph.links)
          .start();

      var link = svg.selectAll(".link")
          .data(graph.links)
        .enter().append("line")
          .attr("class", "link")
          .style("stroke-width", function(d) { return Math.sqrt(d.value); });

      var node = svg.selectAll(".node")
          .data(graph.nodes)
          .enter().append("g")
          .attr("class", "node")
          .call(force.drag);
      
      node.append("circle")
      .attr("r", function(d) { return linearScale(d.size);})
      .style("fill", function(d) { return color(d.group); })
      .on("click", function(d){
    	  d3.selectAll('circle').attr("class", "off");
    	  d3.select(this)
    	  	.attr("class", "active")
    	  content(d.type, d.tid, d.parent_type, d.parent_tid);
      })
      .on("mouseover", function(){d3.select(this).style("fill", "#999");})
      .on("mouseout", function(d) {
    	  if (d.style == 'filled') { d3.select(this).style("fill",color(d.group)); }
    	  else {
    		  d3.select(this).style("fill",color(d.group));
    	  } });
      
      node.append("text")
		.attr("text-anchor", "middle")
		.attr("y", function(d) { return  "-" + linearScale(d.size) + "px";})
		//.attr("fill", "#000")
		.text( function(d) { return d.name } ) ;
      
      node.append("text")
		.attr("text-anchor", "middle")
		.text( function(d) { return d.size } ) ;
      
      force.on("tick", function() {
        link.attr("x1", function(d) { return d.source.x; })
            .attr("y1", function(d) { return d.source.y; })
            .attr("x2", function(d) { return d.target.x; })
            .attr("y2", function(d) { return d.target.y; });

        node.attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")";});
        
      });
    });
    	}
    	function content(type, tid, parent_type, parent_tid){
    		var uri = settings.basePath + 'd3t/content';
    		$( "#graph-content-container" ).hide();
    		$.get( uri, {
				type: type, 
				tid: tid,
				parent_type: parent_type,
				parent_tid: parent_tid
			}, function( data ) {
    			  $( "#graph-content-container" ).html( data ).fadeIn( "slow" );
    			});
    	}
    }
  };
})(jQuery);