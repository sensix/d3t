<?php 

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
/**
 * Costruisce l'oggetto json con i dati
 *
 */
function d3t_data_json(){

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

function _get_nodes_related_to_terms($mappa_tematica_tid, $disciplina_tid){
	$entities = array();
	$bundles = array('documento');
	$nodeQuery = new EntityFieldQuery();
	$result = $nodeQuery->entityCondition('entity_type', 'node')
	->propertyCondition('status', 1)
	->entityCondition('bundle', $bundles)
	->fieldCondition('field_disciplina', 'tid', $disciplina_tid)
	->fieldCondition('field_mappa_tematica', 'tid', $mappa_tematica_tid)
	->execute();

	if (!empty($result['node'])) {
		$entities = entity_load('node', array_keys($result['node']));
	}
	return $entities;
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

