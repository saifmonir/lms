<?php
/**
  *
**/
function assignment_home($arg1='',$arg2='',$arg3=''){

    drupal_add_css(drupal_get_path('module','dcc_assignment').'/css/dcc_assignment.css');
    global $user;
	
    $output = '';	
    $form='';

	switch($arg1){
		case 'list':
			$output .= assignment_list();	
		break;
		case 'view':
			$output .= assignment_view($arg2);	
		break;
		case 'submit':
			$form =drupal_get_form('assignment_submit_form','add',$arg2,$arg3);
			$output .= drupal_render($form);	
		break;
		case 'submit-list':
			$output .= assignment_submit_list($arg2);	
		break;
		case 'submit-view':
			$output .= assignment_submit_view($arg2);	
		break;
		default:
			$output .= assignment_list();	
		break;
	}	
	
	return $output;
}

/**
  *
**/
function assignment_list($arg2='',$arg3=''){
    global $user;
	$batchA = getCourse();
    $fbatch = '';
    $fsem = '';
    $qry = '';
	if(isset($_GET['batch'])){
		if(is_numeric($_GET['batch'])){
			$fbatch = $_GET['batch'];
			$qry .= " AND ag.`batch` = '$fbatch'";
		}
	}
	if(isset($_GET['sem'])){
		if(is_numeric($_GET['sem'])){
			$fsem = $_GET['sem'];
			$qry .= " AND ag.`sem` = '$fsem'";
		}
	}
    $batch = (isset($batchA[$fbatch]))?' | '.$batchA[$fbatch]:'';
	$barr = array(
        t('Home') => '<front>'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	drupal_set_title('Assignment List'.$batch);
    $uid = $user->uid;
    $num_per_page = 50;
    $page = 0;
	if(isset($_GET['page']) && is_numeric($_GET['page'])){
		$page = $_GET['page'];
		$offset = $num_per_page * $page;
	}else{
		$offset = 0;
	}
    
	
    $cquery = db_query("SELECT COUNT(*) as total FROM {assignment} as ag LEFT JOIN {users} as u ON ag.uid = u.uid WHERE 1=1 $qry;");
    $result = db_query("SELECT ag.*,u.name as uname FROM {assignment} as ag LEFT JOIN {users} as u ON ag.uid = u.uid WHERE 1=1 $qry ORDER BY ag.created DESC, ag.`batch` DESC, ag.`weight` ASC LIMIT $offset,$num_per_page");
	
	$output ='';
	$rows = array();	
    $i = $offset +1;
	$edate = date('Y-m-d');
	$etime = strtotime($edate);
	if ($result->rowCount()>0) {
		foreach ($result as $row) {
            $batch = (isset($batchA[$row->batch]))?$batchA[$row->batch]:$row->batch;
            $scount = db_query("SELECT COUNT(*) as total FROM {assignment_submit} as ags WHERE asid='$row->asid';")->fetchField();
            $slnk = l('Submit','assignment/submit/'.$row->asid,array('query'=>array('destination'=>'assignment/list'),'attributes'=>array('class'=>array('view'),'title'=>'View')));
			$ldate = date('Y-m-d',$row->ldate)." 11:59 pm";
			$ltime = strtotime($ldate);
			if($ltime < $etime) $slnk = '<span class="red">Expired</span>';
            $rows[] = array(
				'data' => array(
					$i,
                    l($row->title,'assignment/view/'.$row->asid,array('query'=>array('destination'=>'assignment/list'),'attributes'=>array('title'=>'View'))),
                    substr($batch,0,4),
					date('j M, Y',$row->ldate),
					$row->uname,
                    l($scount,'assignment/submit-list/'.$row->asid,array('query'=>array('destination'=>'assignment/list'),'attributes'=>array('class'=>array('view'),'title'=>'View'))),
					$slnk,
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
		t('Teacher'),				
		t('Std'),				
		t('Action'),		
	);
	
	$variables['header'] =$header;
	$variables['rows'] =$rows;
	$variables['attributes'] = '';
	$bArr = getCourse();
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
			<div class="filter-row"><label>Batch:</label><select name="batch">'.$bList.'</select></div>
			<div class="filter-row"><input type="submit" class="button btn-small" name="filter" value="Filter" /></div>
		</form>
		<div class="clear"></div>
		</div>
	</fieldset>';
		
	$output .= theme('table',$variables);	
    
    $total = $cquery->fetchField();
    pager_default_initialize($total,$num_per_page);
    $output .= theme('pager');
        
	db_close(); return $output;	
}

/**
  *
**/
function assignment_view($asid=''){
	global $user;
	$barr = array(
        t('Home') => '<front>',
		t('Assignment List') => 'assignment/list'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	$output = '';
	$output .= '<div class="aspage">';
	if(is_numeric($asid)){
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid ', array(':asid' => $asid))->fetchObject(); 
		if(isset($row->asid)){
			$bArr = getCourse();
			$batch = (isset($bArr[$row->batch]))?$bArr[$row->batch]:$row->batch;
			drupal_set_title($row->title);
			$ldate = date('Y-m-d',$row->ldate);
			$output .= '<div class="row submitted">'.t('Posted on').': '.date('d M, Y',$row->created).'</div>';
			$output .= '<div class="row"><label>'.t('Title').':</label>'.$row->title.'</div>';
			$output .= '<div class="row"><label>'.t('Course').':</label>'.$batch.'</div>';
			$output .= '<div class="row"><label>'.t('Last date of submission').':</label>'.$ldate.'</div>';
			$output .= '<div class="row"><label>'.t('Attachment').':</label>'.file_view($row->fid,'html').'</div>';
			$output .= '<div class="row"><label>'.t('Instructions').':</label>'.$row->desc.'</div>';
			$output .= '<div class="row"><label>'.t('Status').':</label>'.$row->status.'</div>';

		}
	}else{
		drupal_set_title('Assignment Details');
	}
	
	$output .= '<div class="clear">'.l('Back to list','assignment/list/',array('attributes'=>array('class'=>array('button')))).'</div>';
	$output .= '</div>';
	return $output;
}

/**
* Salary of Accounts list function
*/
function assignment_submit_list($asid='') {
    global $user;
	$barr = array(
        t('Home') => '<front>',
        t('Assignment List') => 'assignment/list'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
    $fasid = $asid;
    $qry = '';
	if(isset($_GET['asid'])){
        if(is_numeric($_GET['asid'])){
            $fasid = $_GET['asid'];
            $qry .= " AND `asid` = '$asid'";
        }
    }
    $atitle = '';
	if(is_numeric($fasid)){ 
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid', array(':asid' => $fasid))->fetchObject(); 
		if(isset($row->asid)){
			$atitle = $row->title;
		}
	}
	drupal_set_title('Submission List: '.$atitle);
    $uid = $user->uid;
    $num_per_page = 50;
    $page = 0;
	if(isset($_GET['page']) && is_numeric($_GET['page'])){
		$page = $_GET['page'];
		$offset = $num_per_page * $page;
	}else{
		$offset = 0;
	}
    
    $result = db_query("SELECT ass.*,u.name FROM {assignment_submit} as ass LEFT JOIN {users} as u ON ass.uid = u.uid WHERE asid='$fasid' ORDER BY ass.`updated` ASC LIMIT $offset,$num_per_page");
    $cquery = db_query("SELECT COUNT(*) as total FROM {assignment_submit} WHERE asid='$fasid' ;");
	
	$output ='';$rows = array();	
    $i = $offset +1;
	
	$stArr = getAssignmentSt();
	if ($result->rowCount()>0) {
		foreach ($result as $row) {
			$status = (isset($stArr[$row->status]))?$stArr[$row->status]:$row->status;
           $rows[] = array(
				'data' => array(
					$i,
					$row->name,
					date('j M, Y',$row->created),
					$status,
					$row->comment,
					l('View','assignment/submit-view/'.$row->asid2,array('query'=>array('destination'=>'assignment/submit-list/'.$row->asid),'attributes'=>array('class'=>array('view'),'title'=>'View'))),
				)
			);		
            $i++;
		}
	}
	
	$header = array (
		t('S.l'),		
		t('Student'),				
		t('Date'),				
		t('Status'),				
		t('Reply'),				
		t('Action'),		
	);
	
	$variables['header'] =$header;
	$variables['rows'] =$rows;
	$variables['attributes'] = '';
	
	$aList = '<option value=""> --'.t('Select one').'-- </option>';
	$result = db_query("SELECT asid,title FROM {assignment}");
	if ($result->rowCount()>0) {
		foreach ($result as $row) {
			$selected = '';
			if($row->asid == $fasid) $selected = 'selected = "selected"';
			$aList .= '<option value="'.$row->asid.'" '.$selected.'>'.$row->title.'</option>'; 
		}
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
			<div class="filter-row"><label>Assignment:</label><select name="asid">'.$aList.'</select></div>
			<div class="filter-row"><input type="submit" class="button btn-small" name="filter" value="Filter" /></div>
		</form>
		<div class="clear"></div>
		</div>
	</fieldset>';
	
	$output .= theme('table',$variables);	
    
    $total = $cquery->fetchField();
    pager_default_initialize($total,$num_per_page);
    $output .= theme('pager');
        
	db_close(); return $output;	
}

/**
* assignment Form
*/
function assignment_submit_form($form, &$form_state,$action, $asid='', $asid2=''){
	global $user;
	
	if(!$user->uid){
		drupal_set_message("Access Denied: Login to submit your assignment!",'warning');
		$dest = drupal_get_destination();
		drupal_goto('user/login'); 
	}
	$barr = array(
        t('Home') => '<front>',
        t('Assignment List') => 'assignment/list'
    );
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
    $atitle = '';
	if(is_numeric($asid)){ 
		$row = db_query('SELECT * FROM {assignment} c WHERE c.asid = :asid', array(':asid' => $asid))->fetchObject(); 
		if(isset($row->asid)){
			$atitle = $row->title;
            if($row->ldate < time()) {
                drupal_set_message("Submission date expired!",'warning');
                drupal_goto('assignment/list'); 
            }
		}
	}
	drupal_set_title('Assignment: '.$atitle);
	//drupal_add_js(drupal_get_path('module','dcc_assignment').'/js/dcc_assignment_ck.js');
	$form = array();
	$form['#attributes'] = array('enctype' => "multipart/form-data");
	
	$uid = $user->uid;
	$status = '2';
	$asid2 = '';
	$desc = '';
	$type = 'assignment';
	$fid = '';
	
	if(is_numeric($asid2)){ 
		$row = db_query('SELECT * FROM {assignment_submit} c WHERE c.asid2 = :asid2', array(':asid2' => $asid2))->fetchObject(); 
		if(isset($row->asid2)){
			$status = $row->status;
			$desc = $row->desc;
			$fid = $row->fid;
			
			$form['asid2'] = array(
				'#type' => 'hidden',
				'#default_value' => $row->asid2
			);
		}
	}
	$form['status'] = array(
		'#type' => 'hidden',
		'#default_value' => $status
	);
	$form['asid'] = array(
		'#type' => 'hidden',
		'#default_value' => $asid
	);
	$form['uid'] = array(
		'#type' => 'hidden',
		'#default_value' => $uid
	);
	$form['attachment'] = array(
		'#type' => 'file',
		'#title' => t('Source Code file:'),
		'#size' => 20, 
		'#description' => t('Max upload size 2MB'),
		'#suffix'=>file_view($fid,'html')  
	);
	$form['desc'] = array(
		'#type'=>'text_format',
		'#format'=>'full_html',
		'#title' => t('Source Code text:'),	
		'#description' => '',		 
		'#default_value' => $desc,
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
function assignment_submit_form_validate($form, &$form_state){
}

/**
* assignment save 
*/
function assignment_submit_form_submit($form, &$form_state){ 
	$data = array();

	$values = $form_state['values'];
	$input = $form_state['input'];
	$fid = 0;
	$data['desc'] = addslashes($values['desc']['value']);
	$data['asid'] = (int)$values['asid'];
	$data['uid'] = (int)$values['uid'];
	$data['status'] = (int)$values['status'];
	$data['fid'] = 0;
	$fid = 0;
	if(isset($values['asid2'])){
		$row = db_query('SELECT * FROM {assignment_submit} c WHERE c.asid2 = :asid2', array(':asid' => $values['asid2']))->fetchObject();
		$fid = $row->fid;
	}
    $pfid = upload_file('attachment','assignment','file',$fid);
	if($pfid){
		$data['fid'] = $pfid;
	}
	$data['updated'] = time();
	if($data['fid'] >0 || strlen($data['desc']) > 10){
		if(isset($values['asid2'])){
			$asid2 = $values['asid2'];
			db_query("UPDATE {assignment_submit} SET `asid`='".$data['asid']."', `desc`='".$data['desc']."', `fid`='".$data['fid']."', `updated`='".$data['updated']."' WHERE asid2='$asid2';");
			//db_query('INSERT INTO {assignment_submit} (`asid`,`desc`, `fid`, `updated`, `created`, `uid`, `status`) VALUES (:asid,:d1,:fid,:t1,:t2,:uid,:status)', array(':asid'=>$data['asid'],':d1'=>$data['desc'],':fid'=>$data['fid'],':t1'=>$data['updated'],':t2'=>$data['created'],':uid'=>$data['uid'],':status'=>$data['status']));
			/*db_update('assignment_submit')
			->condition('asid2', $asid2)
			->fields($data);*/
			drupal_set_message(t('Assignment updated successfully .'),'status');
		}else{
			$data['created'] = time();
			db_query('INSERT INTO {assignment_submit} (`asid`,`desc`, `fid`, `updated`, `created`, `uid`, `status`) VALUES (:asid,:d1,:fid,:t1,:t2,:uid,:status)', array(':asid'=>$data['asid'],':d1'=>$data['desc'],':fid'=>$data['fid'],':t1'=>$data['updated'],':t2'=>$data['created'],':uid'=>$data['uid'],':status'=>$data['status']));
			/*$asid2 = db_insert('assignment_submit')
				->fields($data)
				->execute();*/
			drupal_set_message(t('Assignment successfully submited.'),'status');
		}
	}else{
		drupal_set_message(t('Blank submission! Provide file or text.'),'warning');
	}
	
	drupal_goto('assignment/submit-list');
}

/**
  *
**/
function assignment_submit_view($asid2=''){
	global $user;
    $asid = 0;
	
	if(!in_array('teacher',$user->roles) && !in_array('admin',$user->roles)){
		drupal_set_message("You are not authorized to access this page.",'warning');
		drupal_goto('assignment/submit-list/'.$asid); 
	}
	if(is_numeric($asid2)){
		 if(isset($_POST['st'])){
			$st = (int)$_POST['st'];
			$comment = addslashes($_POST['comment']);
			db_query("UPDATE {assignment_submit} SET status='$st', comment='$comment', uid2='$user->uid' WHERE asid2 ='$asid2'");
		}
		$row = db_query('SELECT * FROM {assignment_submit} c WHERE c.asid2 = :asid2', array(':asid2' => $asid2))->fetchObject(); 
        if(isset($row->asid)) $asid = $row->asid;
	}
    $barr = array(
        t('Home') => '<front>',
        t('Assignment List') => 'assignment/list',
		t('Assignment Submission') => 'assignment/submit-list/'.$asid
    );
	drupal_set_title('Submission details');
	if(function_exists("set_breadcrumb")) set_breadcrumb($barr);
	$output = '';
	$output .= '<div class="aspage">';
	$stArr = getAssignmentSt();
	if(isset($row->asid2)){
		$status = (isset($stArr[$row->status]))?$stArr[$row->status]:$row->status;
        $ul = user_load($row->uid);
        $output .= '<div class="row submitted">'.t('Posted on').': '.date('d M, Y',$row->created).'</div>';
        $output .= '<div class="row"><label>'.t('Submitted By').':</label>'.$ul->name.'</div>';
        $output .= '<div class="row"><label>'.t('Source Code File').':</label>'.file_view($row->fid,'html').'</div>';
        $output .= '<div class="row"><label>'.t('Source Code Text').':</label>'.$row->desc.'</div>';
        if($row->status == '2'){
			$stList = '<select name="st">';
			foreach($stArr as $k=>$v){
				$stList .= '<option value="'.$k.'">'.$v.'</option>';
			}
			$stList .= '</select>';
			$output .= '<div class="row"><label>'.t('Update Status').':</label><form method="post"><input type="text" name="comment" value="Checked" placeholder="Your comment here" size="60"/><br/>'.$stList.' <br/><input type="submit" class="button" value="Update" /></form></div>';
		}else{
			$output .= '<div class="row"><label>'.t('Status').':</label>'.$status.'</div>';
		}

	}else{
		drupal_set_title('Submission Details');
	}
	
	$output .= '<div class="clear">'.l('Back to list','assignment/submit-list/'.$asid,array('attributes'=>array('class'=>array('button')))).'</div>';
	$output .= '</div>';
	return $output;
}
