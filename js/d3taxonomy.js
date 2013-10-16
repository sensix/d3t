(function($){
  Drupal.d3t = {
    w: 960,
    h: 640,
  };
  Drupal.d3t.node;
  Drupal.d3t.link;
  Drupal.d3t.root;
  Drupal.d3t.update = function() {
    var nodes = Drupal.d3t.flatten(Drupal.d3t.root),
    links = d3.layout.tree().links(nodes);

    // Restart the force layout.
    Drupal.d3t.force
      .nodes(nodes)
      .links(links)
      .start();

    // Update the links…
    Drupal.d3t.link = Drupal.d3t.vis.selectAll("line.link")
      .data(links, function(d) { return d.target.id; });

    // Enter any new links.
    Drupal.d3t.link.enter().insert("svg:line", ".node")
      .attr("class", "link")
      .attr("x1", function(d) { return d.source.x; })
      .attr("y1", function(d) { return d.source.y; })
      .attr("x2", function(d) { return d.target.x; })
      .attr("y2", function(d) { return d.target.y; });

    // Exit any old links.
    Drupal.d3t.link.exit().remove();

    // Update the nodes…
    Drupal.d3t.node = Drupal.d3t.vis.selectAll("circle.node")
      .data(nodes, function(d) { return d.id; })
      .style("fill", Drupal.d3t.color);

    Drupal.d3t.node.transition()
      .attr("r", function(d) { return Math.sqrt(d.size); });

  	// Enter any new nodes.
  	Drupal.d3t.node.enter().append("svg:circle")
      .attr("class", "node")
      .attr("cx", function(d) { return d.x; })
      .attr("cy", function(d) { return d.y; })
      .attr("r", function(d) { 
    	  return Math.sqrt(d.size); 
    	  })
      .style("fill", Drupal.d3t.color)
      .on("click", Drupal.d3t.click)
      .call(Drupal.d3t.force.drag);

    // Exit any old nodes.
  	Drupal.d3t.node.exit().remove();
  };

  Drupal.d3t.tick = function() {
    Drupal.d3t.link.attr("x1", function(d) { return d.source.x; })
      .attr("y1", function(d) { return d.source.y; })
      .attr("x2", function(d) { return d.target.x; })
      .attr("y2", function(d) { return d.target.y; });

    Drupal.d3t.node.attr("cx", function(d) { return d.x; })
      .attr("cy", function(d) { return d.y; });
  };

  //Color leaf nodes orange, and packages white or blue.
  Drupal.d3t.color = function(d) {
    return d._children ? "#3182bd" : d.children ? "#c6dbef" : "#fd8d3c";
  };

  //Toggle children on click.
  Drupal.d3t.click = function(d) {
    if (d.children) {
      d._children = d.children;
      d.children = null;
    } else {
      d.children = d._children;
      d._children = null;
    }
    Drupal.d3t.update();
  };

  //Returns a list of all nodes under the root.
  Drupal.d3t.flatten = function(root) {
    var nodes = [], i = 0;

    function recurse(node) {
      if (node.children) node.size = node.children.reduce(function(p, v) { return p + recurse(v); }, node.size);
      if (!node.id) node.id = ++i;
      nodes.push(node);
      return node.size;
    }

    root.size = recurse(root);
    return nodes;
  };
  
  Drupal.behaviors.d3t = {
		    attach: function(context, settings){


		      
		      Drupal.d3t.force = d3.layout.force()
		        .on("tick", Drupal.d3t.tick)
		        .charge(function(d) { return d._children ? -d.size / 100 : -30; })
		        .linkDistance(function(d) { return Math.sqrt(d.size) / 10 })
		        .size([Drupal.d3t.w, Drupal.d3t.h - 160]);

		      Drupal.d3t.vis = d3.select("#d3t_pane").append("svg:svg")
		        .attr("width", Drupal.d3t.w)
		        .attr("height", Drupal.d3t.h);

		      d3.json(settings.basePath + "d3t/data.json?vid=5&tid=44", function(error, json) {
		        if(error){
		          alert(error);
		        }
		        else{
		          Drupal.d3t.root = json;
		          Drupal.d3t.root.fixed = true;
		          Drupal.d3t.root.x = Drupal.d3t.w / 2;
		          Drupal.d3t.root.y = Drupal.d3t.h / 2 - 80;
		          Drupal.d3t.update();
		        }
		      });
		    }
		  };

})(jQuery);