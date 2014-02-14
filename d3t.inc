<?php 
function d3t_content(){
	$parent_type = !empty($_REQUEST['parent_type'])?$_REQUEST['parent_type']:false;
	$parent_tid = !empty($_REQUEST['parent_tid'])?$_REQUEST['parent_tid']:false;
	$type = $_REQUEST['type'];
	$tid = $_REQUEST['tid'];
	if($parent_type && $parent_tid){
		$params = array(
				$parent_type => $parent_tid,
				$type => $tid
				);
	}
	else{
		$params = array($type => $tid);
	}
	$nodes = _get_nodes_related_to_terms($params);
	$out = '<ul calss="graph-content">';
	foreach($nodes as $node){
		$out .= '<li>'.l($node->title, 'node/' . $node->nid).'</li>';
	}
	$out .= '</ul>';
	print $out;
}

function d3t_graph_discipline($mappa_tematica_tid){
	$nodes =$links = array();
	$d = d3t_data_discipline_json($mappa_tematica_tid);
	$nodes[] = _get_term_name($mappa_tematica_tid);
	$sum = 0;
	$group = 2;
	foreach($d as &$n){
		$n->parent_type = 'M';
		$n->parent_tid = $mappa_tematica_tid;
		$n->group=$group++;
		$n->type="D";
		$nodes[] = $n;
		$sum += $n->size;
	}
	$nodes[0]->size = $sum;
	$nodes[0]->parent_type = 'ROOT';
	$nodes[0]->parent_tid = 0;
	$nodes[0]->group = 1;
	$nodes[0]->type = 'M';
	for($i=1; $i<count($nodes); $i++){
		$links[] = array("source" => $i, "target" => 0, "value" => $nodes[$i]->size);
	}
	$data = array('nodes' => $nodes, "links" => $links);
	drupal_json_output($data);
}

function d3t_graph_mappe_tematiche($disciplina_tid){
	$nodes =$links = array();
	$d = d3t_data_areatematica_json($disciplina_tid);
	$nodes[] = _get_term_name($disciplina_tid);
	$sum = 0;
	$group = 2;
	foreach($d as &$n){
		$n->parent_type = 'D';
		$n->parent_tid = $disciplina_tid;
		$n->group=$group++;
		$n->type="mappa-tematica";
		$nodes[] = $n;
		$sum += $n->size;
	}
	$nodes[0]->size = $sum;
	$nodes[0]->parent_type = 'ROOT';
	$nodes[0]->parent_tid = 0;
	$nodes[0]->group = 1;
	$nodes[0]->type = 'D';
	for($i=1; $i<count($nodes); $i++){
		$links[] = array("source" => $i, "target" => 0, "value" => $nodes[$i]->size);
	}
	$data = array('nodes' => $nodes, "links" => $links);
	drupal_json_output($data);	
}

function _get_term_name($tid){
	return db_query("select name,0 as size,tid from taxonomy_term_data term
	where term.tid=:tid",array(":tid" => $tid))->fetch();
}
/**
 * Costruisce l'oggetto json con i dati sulle discipline, avendo fissato l'area tematica
 *
 */
function d3t_data_discipline_json($mappa_tematica_tid){
	return db_query("select t.name, count(node.nid) as size, t.tid from node
join field_data_field_disciplina d on d.entity_id=node.nid
join field_data_field_mappa_tematica m on m.entity_id=node.nid
join taxonomy_term_data t on t.tid=d.field_disciplina_tid
where m.field_mappa_tematica_tid=:mappa_tematica_tid
group by t.tid, t.name;", array(':mappa_tematica_tid' => $mappa_tematica_tid))->fetchAll();
}
/**
 * Costruisce l'oggetto json con i dati sulle aree tematiche, avendo fissato la disciplina
 *
 */
function d3t_data_areatematica_json($disciplina_tid){
	
	return db_query("select t.name, count(node.nid) as size,t.tid from node
join field_data_field_disciplina d on d.entity_id=node.nid
join field_data_field_mappa_tematica m on m.entity_id=node.nid
join taxonomy_term_data t on t.tid=m.field_mappa_tematica_tid
where d.field_disciplina_tid=:disciplina_tid
group by t.tid, t.name;", array(':disciplina_tid' => $disciplina_tid))->fetchAll();
	
	
}
function _get_nodes_for_discilpina(){
	db_query("select node.nid,node.title,node.type,t.name,t.tid from node
join field_data_field_disciplina d on d.entity_id=node.nid
join field_data_field_mappa_tematica m on m.entity_id=node.nid
join taxonomy_term_data t on t.tid=m.field_mappa_tematica_tid
where d.field_disciplina_tid=:tid;", array(':tid' => $disciplina_nid));
}

/*
 * Recupera le mappe tematiche
 */
function d3t_aree_tematiche_json(){
	$items =  db_query("select t.tid,t.name from taxonomy_vocabulary v
join taxonomy_term_data t on t.vid=v.vid
where v.machine_name=:machine_name;", array(':machine_name' => 'mappe_tematiche'))->fetchAll();
	drupal_json_output($items);
}
/*
 * Recupera le discipline
 */
function d3t_discipline_json(){
	$items = db_query("select t.tid,t.name from taxonomy_vocabulary v
join taxonomy_term_data t on t.vid=v.vid
where v.machine_name=:machine_name;", array(':machine_name' => 'discipline'))->fetchAll();
	drupal_json_output($items);
	
}


function d3t_test_data_json(){
	global $base_url;
	$vocabularies = _filter_vocabularies(array('mappe_tematiche', 'discipline'));
	$terms = array();
	$nodes = array();
	$g_nodes = array();
	$g_links = array();
	$colors = array('mappe_tematiche' => '#b94431', 'discipline' => '#da991c');
	$size = array('mappe_tematiche' => 6, 'discipline' => 4);
	
	
	foreach($vocabularies as $vocabulary){
		$terms = _get_vocabolary_terms($vocabulary->machine_name);
		foreach($terms as $term){
			array_push($g_nodes, (object)array(
				'_tid_' => $term->tid, 
				'color' => $colors[$term->machine_name], 
				'id' => $term->name, 
				'size' => $size[$term->machine_name],
				//'URI' => url($base_url . '/term/' . $term->tid),
			));
		}
	}

	$nodes = _filter_nodes();
	foreach($nodes as $node){
		$link = array();
		if(!empty($node->field_disciplina_tid)){
			$link[] = $node->field_disciplina_tid;
		}
		if(!empty($node->field_mappa_tematica_tid)){
			$link[] = $node->field_mappa_tematica_tid;
		}
		array_push($g_nodes, (object)array(
			'_link_' => $link,  
			'id' => $node->title,
			//'URI' => url($base_url . '/content/' . $node->nid)
		));
	}
	foreach($g_nodes as $key => $node){
		if(isset($node->_link_)){
			foreach($node->_link_ as $tid){
				$target = _get_index_tid($g_nodes, $tid);
				if($target){
					array_push($g_links, (object)array('color' => '#b94431', 'source' => $key, 'target' => $target));
				}
			}
		}
	}
	
	foreach($g_nodes as $node){
		if(isset($node->_tid_)){
			unset($node->_tid_);
		}
		if(isset($node->_link_)){
			unset($node->_link_);
		}
		
	}
	
	$data = new stdClass;
	$data->directed = true;
	$data->graph = array();
	$data->nodes = $g_nodes;
	$data->links = $g_links;
	$data->multigraph = false;
	drupal_json_output($data);
}

function _get_index_tid($g_nodes, $tid){
	foreach($g_nodes as $key => $node){
		if($node->_tid_ == $tid){
			return $key;
		}
	}
	return false;
}

function _filter_nodes(){
	return db_query("select node.*,d.field_disciplina_tid,m.field_mappa_tematica_tid 
from node
left join field_data_field_disciplina d on d.entity_id=node.nid
left join field_data_field_mappa_tematica m on m.entity_id=node.nid
where 
(m.field_mappa_tematica_tid is not null or d.field_disciplina_tid is not null)
and node.status=1")->fetchAllAssoc('nid');
}

function _filter_vocabularies($filter){
	$vocabularies = array();
	$vocabularies_all = taxonomy_get_vocabularies();
	foreach($filter as $v_use){
		foreach($vocabularies_all as $v_all){
			if($v_all->machine_name == $v_use){
				array_push($vocabularies, $v_all);
			}
		}
	}
	return $vocabularies;
}

function _d3t_test_data_json(){
	// create curl resource
	$ch = curl_init();
	// set url
	curl_setopt($ch, CURLOPT_URL, "http://graus.nu/d3/graph.json");
	//return the transfer as a string
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	// $output contains the output string
	$output = curl_exec($ch);
	// close curl resource to free up system resources
	curl_close($ch);
	drupal_add_http_header('Content-Type', 'application/json');
	print $output;
	exit;
}


function aaa_d3t_data_json(){

	$vid = $_GET['vid'];
	$tid = $_GET['tid'];

	$data = new Bubble('root');
	// palla iniziale, un'area tematica
	$term_mappa_tematica = get_term($vid, $tid);
	$data->name = $term_mappa_tematica->name;
	$data->type = 'mappa_tematica';
	$data->size = 1000;

	//recupera tutti i termini disciplina
	$terms_discipline = _get_vocabolary_terms('discipline');
	foreach($terms_discipline as $term_disciplina){
		$disciplina = new Bubble($term_disciplina->tid);
		$disciplina->name = $term_disciplina->name;
		$disciplina->link = 'term/' . $term_disciplina->tid;
		$disciplina->type = 'disciplina';
		$disciplina->size = 500;
		// recupera tutti i documenti che appartengono alla mappa tematica e alla disciplina
		$nodes = _get_nodes_related_to_terms($term_mappa_tematica->tid, $term_disciplina->tid);
		foreach($nodes as $node){
			$leaf = new Bubble($node->nid);
			$leaf->type = $node->type;
			$leaf->name = $node->title;
			$leaf->size = 100;
			$leaf->link = 'content/' . $node->nid;
			$disciplina->children[] = $leaf;
		}
		$data->children[] = $disciplina;
	}
	drupal_json_output($data);
}

function d3t_view_graph(){
	$block = module_invoke('d3t', 'block_view', 'graph');
	return render($block['content']);
}

function get_term($vid, $tid){
	return db_query("select * from taxonomy_term_data term
where term.tid=:tid and term.vid=:vid",array(':tid' => $tid, ':vid' => $vid))->fetchObject();
}

function _get_nodes_related_to_terms($params){
	if(count($params) > 1){
		$nodes =  db_query("select node.nid,node.title,node.type from node
join field_data_field_disciplina d on d.entity_id=node.nid
join field_data_field_mappa_tematica m on m.entity_id=node.nid
where d.field_disciplina_tid=:disciplina_tid
and m.field_mappa_tematica_tid=:mappa_tematica_tid
and node.status=1;", array(':disciplina_tid' => $params['D'], ':mappa_tematica_tid' => $params['M']))
		->fetchAll();
	}else{
		reset($params);
		$type = key($params);
		switch($type){
			case "D":
				$nodes =  db_query("select node.nid,node.title,node.type from node
	join field_data_field_disciplina d on d.entity_id=node.nid
	where d.field_disciplina_tid=:disciplina_tid
	and node.status=1;", array(':disciplina_tid' => $params[$type]))
				->fetchAll();
				break;
			case "M":
				$nodes =  db_query("select node.nid,node.title,node.type from node
	join field_data_field_mappa_tematica m on m.entity_id=node.nid
	where m.field_mappa_tematica_tid=:mappa_tematica_tid
	and node.status=1;", array(':mappa_tematica_tid' => $params[$type]))
				->fetchAll();
				break;
		}
	}
	
	return $nodes;
}

function _get_vocabolary_terms($machine_name){
	$terms = array();
	$ret = db_query("select t.vid,v.machine_name,t.tid,t.name from taxonomy_term_data t
join taxonomy_vocabulary v on v.vid=t.vid
where v.machine_name=:machine_name
order by t.weight asc",array(':machine_name' => $machine_name));

	foreach($ret as $row){
		$terms[] =$row;
	}
	return $terms;
}

