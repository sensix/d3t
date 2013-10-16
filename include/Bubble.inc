<?php 

class Bubble{
	public $id = null;
	public $name = '';
	public $type = '';
	public $link = '';
	public $size = 20;
	public $children = array();

	function __construct($id){
		$this->id = $id;
	}
}