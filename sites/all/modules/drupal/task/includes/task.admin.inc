<?php

/**
 * Generates the task type editing form.
 */
function task_type_form($form, &$form_state, $task_type, $op = 'edit') {

  if ($op == 'clone') {
    $task_type->label .= ' (cloned)';
    $task_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $task_type->label,
    '#description' => t('The human-readable name of this task type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($task_type->type) ? $task_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $task_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'task_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this task type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($task_type->description) ? $task_type->description : '',
    '#description' => t('Description about the task type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save task type'),
    '#weight' => 40,
  );

  if (!$task_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete task type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('task_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Submit handler for creating/editing task_type.
 */
function task_type_form_submit(&$form, &$form_state) {
  $task_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  task_type_save($task_type);

  // Redirect user back to list of task types.
  $form_state['redirect'] = 'admin/structure/task-types';
}

function task_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/task-types/' . $form_state['task_type']->type . '/delete';
}

/**
 * Task type delete form.
 */
function task_type_form_delete_confirm($form, &$form_state, $task_type) {
  $form_state['task_type'] = $task_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['task_type_id'] = array('#type' => 'value', '#value' => entity_id('task_type' ,$task_type));
  return confirm_form($form,
    t('Are you sure you want to delete task type %title?', array('%title' => entity_label('task_type', $task_type))),
    'task/' . entity_id('task_type' ,$task_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Task type delete form submit handler.
 */
function task_type_form_delete_confirm_submit($form, &$form_state) {
  $task_type = $form_state['task_type'];
  task_type_delete($task_type);

  watchdog('task_type', '@type: deleted %title.', array('@type' => $task_type->type, '%title' => $task_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $task_type->type, '%title' => $task_type->label)));

  $form_state['redirect'] = 'admin/structure/task-types';
}

/**
 * Page to select task Type to add new task.
 */
function task_admin_add_page() {
  $items = array();
  foreach (task_types() as $task_type_key => $task_type) {
    $items[] = l(entity_label('task_type', $task_type), 'task/add/' . $task_type_key);
  }
  return array('list' => array('#theme' => 'item_list', '#items' => $items, '#title' => t('Select type of task to create.')));
}

/**
 * Add new task page callback.
 */
function task_add($type) {
  $task_type = task_types($type);

  $task = entity_create('task', array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('task_type', $task_type))));

  $output = drupal_get_form('task_form', $task);

  return $output;
}

/**
 * Task Form.
 */
function task_form($form, &$form_state, $task) {
  $form_state['task'] = $task;

  $form['title'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Name'),
    '#default_value' => $task->title,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $task->description,
  );

  $form['completed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Completed'),
    '#default_value' => $task->completed,
  );

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $task->uid,
  );

  field_attach_form('task', $task, $form, $form_state);

  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save task'),
    '#submit' => $submit + array('task_form_submit'),
  );

  // Show Delete button if we edit task.
  $task_id = entity_id('task' ,$task);
  if (!empty($task_id) && task_access('edit', $task)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('task_form_submit_delete'),
    );
  }

  $form['#validate'][] = 'task_form_validate';

  return $form;
}

function task_form_validate($form, &$form_state) {

}

/**
 * Task submit handler.
 */
function task_form_submit($form, &$form_state) {
  $task = $form_state['task'];

  entity_form_submit_build_entity('task', $task, $form, $form_state);

  task_save($task);

  $task_uri = entity_uri('task', $task);

  $form_state['redirect'] = $task_uri['path'];

  drupal_set_message(t('Task %title saved.', array('%title' => entity_label('task', $task))));
}

function task_form_submit_delete($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $task = $form_state['task'];
  $task_uri = entity_uri('task', $task);
  $form_state['redirect'] = array($task_uri['path'] . '/delete', array('query' => $destination));
}

/**
 * Delete confirmation form.
 */
function task_delete_form($form, &$form_state, $task) {
  $form_state['task'] = $task;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['task_type_id'] = array('#type' => 'value', '#value' => entity_id('task' ,$task));
  $task_uri = entity_uri('task', $task);
  return confirm_form($form,
    t('Are you sure you want to delete task %title?', array('%title' => entity_label('task', $task))),
    $task_uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function task_delete_form_submit($form, &$form_state) {
  $task = $form_state['task'];
  task_delete($task);

  drupal_set_message(t('Task %title deleted.', array('%title' => entity_label('task', $task))));

  $form_state['redirect'] = '<front>';
}
