<?php
/**
  *
**/
function assignment_manage($arg1='',$arg2=''){

    global $user;
    drupal_add_css(drupal_get_path('module','dcc_assignment').'/css/dcc_assignment.css');
	
    $output = '';	
    $form='';

	switch($arg1){
		case 'list':
			$output .= assignment_manage_list();	
		break;
		
		case 'add':
            $form =drupal_get_form('assignment_manage_form','add');
			$output .= drupal_render($form);
		break;
		
		case 'edit':
            $form =drupal_get_form('assignment_manage_form','edit', $arg2);
			$output .= drupal_render($form);
		break;
		
		case 'delete':
			$output .= delete_assignment($arg2);	
		break;
        
        default: 
            $output .= assignment_manage_list();	
		break;
		
	}	
	
	return $output;
}

/**
* Salary of Accounts list function
*/
function assignment_manage_list() {
    global $user;
	$barr = array(
        t('Home') => '<front>'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	drupal_set_title('Manage Assignment List');
    $uid = $user->uid;
    $num_per_page =20;
    $page = 0;
	if(isset($_GET['page']) && is_numeric($_GET['page'])){
		$page = $_GET['page'];
		$offset = $num_per_page * $page;
	}else{
		$offset = 0;
	}
    $fbatch = '';
    $fsem = '';
    $qry = '';
	if(isset($_GET['filter'])){
		if(isset($_GET['batch'])){
			if(is_numeric($_GET['batch'])){
				$fbatch = $_GET['batch'];
				$qry .= " AND `batch` = '$fbatch'";
			}
		}
		if(isset($_GET['sem'])){
			if(is_numeric($_GET['sem'])){
				$fsem = $_GET['sem'];
				$qry .= " AND `sem` = '$fsem'";
			}
		}
	}
    $bArr = getCourse();
	$output ='';
	$cquery = db_query("SELECT COUNT(*) as total FROM {assignment} as ag LEFT JOIN {users} as u ON ag.uid = u.uid WHERE ag.uid='$uid' $qry;");
    $result = db_query("SELECT ag.*,u.name as uname FROM {assignment} as ag LEFT JOIN {users} as u ON ag.uid = u.uid WHERE ag.uid='$uid' $qry ORDER BY ag.`updated` DESC LIMIT $offset,$num_per_page");
	$rows = array();	
    $i = $offset +1;
	/*$gArr = getSection();
	$bArr = getTaxonomy(6);
	$sArr = getTaxonomy(3);*/
	if ($result->rowCount()>0) {
		foreach ($result as $row) {
            $course = (isset($bArr[$row->batch]))?$bArr[$row->batch]:$row->batch;
            $rows[] = array(
				'data' => array(
					$i,
					$row->title,
                    $course,
					date('j M, Y',$row->ldate),
					l('Edit','manage/assignment/edit/'.$row->asid,array('query'=>array('destination'=>'manage/assignment/list'),'attributes'=>array('class'=>array('edit'),'title'=>'Edit'))).' | '.l('Submissions','assignment/submit-list/'.$row->asid,array('query'=>array('destination'=>'manage/assignment/list'),'attributes'=>array('class'=>array('view'),'title'=>'View Submissions'))),
				)
			);		
            $i++;
		}
	}
	
	$header = array (
		t('S.l'),		
		t('Title'),				
		t('Course'),			
		t('Last Date'),				
		t('Action'),		
	);
	
	$variables['header'] =$header;
	$variables['rows'] =$rows;
	$variables['attributes'] = '';
	
	$bList = '<option value=""> --'.t('Select one').'-- </option>';
	foreach($bArr as $k => $v){
		$selected = '';
		if($k == $fbatch) $selected = 'selected = "selected"';
		$bList .= '<option value="'.$k.'" '.$selected.'>'.$v.'</option>'; 
	}
	$output .= '
		<fieldset id="filter-block" class="collapsible form-wrapper collapse-processed">
			<legend>
				<span class="fieldset-legend">
					<a class="fieldset-title" display="1" href="javascript:void(0);" onclick="fieldsetShowHide(this)">
						Filter
					</a>
				</span>
			</legend>
		<div id="filter-div" class="filter">
		<form method="GET" action="">
			<div class="filter-row"><label>Course:</label><select name="batch">'.$bList.'</select></div>
			
			<div class="filter-row"><input type="submit" class="button btn-small" name="filter" value="Filter" /></div>
		</form>
		<div class="clear"></div>
		</div>
	</fieldset>';
	
	$output .= '<div class="blocked link-div">'.l('Add new','manage/assignment/add',array('attributes'=>array('class'=>array('button btn-small right')))).'<div class="clear"></div></div>';
	
	$output .= theme('table',$variables);	
    
    $total = $cquery->fetchField();
    pager_default_initialize($total,$num_per_page);
    $output .= theme('pager');
        
	db_close(); return $output;	
}

/**
* assignment Form
*/
function assignment_manage_form($form, &$form_state,$action, $asid=''){
	global $user;
	$barr = array(
        t('Home') => '<front>'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	drupal_set_title('Assignment Form');
	
	//drupal_add_js(drupal_get_path('module','dcc_assignment').'/js/dcc_assignment_ck.js');
	drupal_add_library('system','ui.datepicker');
	drupal_add_js('jQuery(document).ready(function(){jQuery( ".pickadate" ).datepicker({
      dateFormat: "yy-mm-dd",
	  changeYear: false,
      autoSize: true
    });});', 'inline');  
	
	$form = array();
	$form['#attributes'] = array('enctype' => "multipart/form-data");
	
	$uid = $user->uid;
	$status = '1';
	$fid = '';
	$batch = '';
	$sec = '';
	$sem = '';
	$desc = '';
	$weight = '0';
	$type = 'assignment';
	$title = '';
	
    $ldate = date('Y-m-d');
	if(is_numeric($asid)){ 
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid', array(':asid' => $asid))->fetchObject(); 
		if(isset($row->asid)){
			$status = $row->status;
			$title = $row->title;
			$batch = $row->batch;
			$sec = $row->sec;
			$ldate = date('Y-m-d',$row->ldate);
			$desc = $row->desc;
			$weight = $row->weight;
			
			$form['asid'] = array(
				'#type' => 'hidden',
				'#default_value' => $row->asid
			);
		}
	}
	$form['status'] = array(
		'#type' => 'hidden',
		'#default_value' => $status
	);
	$form['uid'] = array(
		'#type' => 'hidden',
		'#default_value' => $uid
	);
	$bArr = array();
	$result = db_query("SELECT * FROM {node} as n WHERE type = 'course' ORDER BY title ASC");
	if ($result->rowCount()>0) {
		foreach ($result as $row) {
				$bArr[$row->nid] = $row->title;
		}
	}
	$form['batch'] = array(
		'#type' => 'select',
		'#title' => t('Course:'),		 
		'#default_value' => $batch,
		'#options' => $bArr, 
	);
	$form['title'] = array(
		'#type' => 'textfield',
		'#title' => t('Title:'),	
		'#description' => '',		 
		'#default_value' => $title,
		'#prefix' => '', 
	);
	$form['ldate'] = array(
		'#type' => 'textfield',
		'#title' => t('Last Date of Submission:'),	
		'#description' => '',		 
		'#default_value' => $ldate,
        '#attributes'=>array('class'=>array('pickadate')),
		'#prefix' => '', 
	);
    $form['attachment'] = array(
		'#type' => 'file',
		'#title' => t('Attachment:'),
		'#description' => t('Upload dont exceed 5MB'),
		'#suffix'=>''//file_view($fid,'html')  
	);
	$form['desc'] = array(
		'#type'=>'text_format',
		'#format'=>'full_html',
		'#title' => t('Instructions:'),	
		'#description' => '',		 
		'#resizable'=>false,
		'#default_value' => $desc,
		'#prefix' => '', 
	);
	
	$form['weight'] = array(
		'#type' => 'textfield',
		'#title' => t('Weight:'),	
		'#description' => 'An interger value (weight) will be used to sort ascending',		 
		'#size' => 3, 
		'#default_value' => $weight,
		'#prefix' => '', 
	);
	$form['Submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save'),
		'#weight' => 100,
		'#attributes' => array('class' => array('button')),
	);
	return $form;
}

/**
* assignment validate
*/
function assignment_manage_form_validate($form, &$form_state){
}

/**
* assignment save 
*/
function assignment_manage_form_submit($form, &$form_state){ 
	$data = array();
	$values = $form_state['values'];
	$input = $form_state['input'];
	$data['title'] = addslashes($values['title']);
	$data['desc'] = addslashes($values['desc']['value']);
	$data['batch'] = (int)$values['batch'];
	$data['sem'] = 0;
	$data['sec'] = 0;
	$data['ldate'] = strtotime($values['ldate']);
	$fid = 0;
	$data['fid'] = 0;
	if(isset($values['asid'])){
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid', array(':asid' => $values['asid']))->fetchObject();
		$fid = $row->fid;
		$data['fid'] = $row->fid;
	}
    $pfid = upload_file('attachment','assignment','file',$fid);
	if($pfid){
		$data['fid'] = $pfid;
	}
    
	$data['updated'] = time();
	$data['uid'] = (int)$values['uid'];
	$data['status'] = (int)$values['status'];
	$data['weight'] = (int)$values['weight'];
    
	if(isset($values['asid'])){
		$asid = $values['asid'];
		db_query("UPDATE {assignment} SET `title`='".$data['title']."', `desc`='".$data['desc']."', `batch`='".$data['batch']."', `sem`='".$data['sem']."',`sec`='".$data['sec']."', `fid`='".$data['fid']."', `ldate`='".$data['ldate']."', `updated`='".$data['updated']."', `status`='".$data['status']."', `weight`='".$data['weight']."' WHERE asid='$asid';");
		/*db_update('assignment')
		->condition('asid', $asid)
		->fields($data);*/
		drupal_set_message(t('Assignment updated successfully.'),'status');
	}else{
		$data['created'] = time();
		db_query("INSERT INTO {assignment} (`title`, `desc`, `batch`, `sem`,`sec`, `fid`, `ldate`, `updated`, `created`, `uid`, `status`, `weight`) VALUES ('".$data['title']."', '".$data['desc']."', '".$data['batch']."', '".$data['sem']."', '".$data['sec']."', '".$data['fid']."', '".$data['ldate']."', '".$data['updated']."', '".$data['created']."', '".$data['uid']."', '".$data['status']."', '".$data['weight']."');");
		/*$asid = db_insert('assignment')
			->fields(array(	
						'asid' => 1,			
						'title' => $data['title'],			
						'desc' => $data['desc'],	
						'batch' => $data['batch'],	
						'sem' => $data['sem'],	
						'ldate' => $data['ldate'],	
						'fid' => $data['fid'],	
						'created' => $data['created'],	
						'updated' => $data['updated'],	   
						'status' => $data['status']			
					))
			->execute();*/
		drupal_set_message(t('Assignment successfully added.'),'status');
	}
	
	drupal_goto('manage/assignment/list');
}

/**
  *
**/
function assignment_manage_view($asid=''){
	global $user;
	$barr = array(
        t('Home') => '<front>',
		t('Manage Assignment List') => 'manage/assignment/list'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	$output = '';
	$output .= '<div class="aspage">';
	if(is_numeric($asid)){
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid', array(':asid' => $asid))->fetchObject(); 
		if(isset($row->asid)){
			$gArr = getSection(3);
			$sArr = getTaxonomy(3);
			$bArr = getTaxonomy(6);
			$batch = (isset($bArr[$row->batch]))?$bArr[$row->batch]:$row->batch;
			$sem = (isset($sArr[$row->sem]))?$sArr[$row->sem]:$row->sem;
			$sec = (isset($gArr[$row->sec]))?$gArr[$row->sec]:$row->sec;
			drupal_set_title($row->title);
			$ldate = date('Y-m-d',$row->ldate);
			$output .= '<div class="row submitted">'.t('Posted on').': '.date('d M, Y',$row->created).'</div>';
			$output .= '<div class="row"><label>'.t('Title').':</label>'.$row->title.'</div>';
			$output .= '<div class="row"><label>'.t('Batch').':</label>'.$batch.'</div>';
			$output .= '<div class="row"><label>'.t('Semester').':</label>'.$sem.'</div>';
			$output .= '<div class="row"><label>'.t('Section/Group').':</label>'.$sec.'</div>';
			$output .= '<div class="row"><label>'.t('Last date of submission').':</label>'.$ldate.'</div>';
			$output .= '<div class="row"><label>'.t('Attachment').':</label>'.file_view($row->fid,'html').'</div>';
			$output .= '<div class="row"><label>'.t('Instructions').':</label>'.$row->desc.'</div>';
			$output .= '<div class="row"><label>'.t('Status').':</label>'.$row->status.'</div>';

		}
	}else{
		drupal_set_title('Assignment Details');
	}
	
	$output .= '<div class="clear">'.l('Back to list','manage/assignment/list/',array('attributes'=>array('class'=>array('button')))).'</div>';
	$output .= '</div>';
	return $output;
}
