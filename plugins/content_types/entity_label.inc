<?php

/**
 * @file
 * Content type plugin to expose rendered entities, view mode configuration
 * still available.
 */

$plugin = array(
  'title' => t('Entity label'),
  'content type' => 'genero_hooks_entity_label_content_type_info',
  'defaults' => array(
    'link' => TRUE,
    'markup' => 'none',
    'id' => '',
    'class' => '',
  ),
);

/**
 * Get the entity content type info.
 */
function genero_hooks_entity_label_content_type_info($entity_type) {
  $types = genero_hooks_entity_label_content_type_content_types();
  if (isset($types[$entity_type])) {
    return $types[$entity_type];
  }
}

/**
 * Implements hook_PLUGIN_content_type_content_types().
 *
 * Rendered entity use entity types machine name as subtype name.
 */
function genero_hooks_entity_label_content_type_content_types() {
  $types = array();
  $entities = entity_get_info();

  foreach ($entities as $entity_type => $info) {
    if (entity_type_supports($entity_type, 'view')) {
      $types[$entity_type] = array(
        'title' => t('@entity_type label', array('@entity_type' => $info['label'])),
        'category' => t('Entity'),
        'required context' => new ctools_context_required(t('Entity'), $entity_type),
      );
    }
  }

  return $types;
}

/**
 * Returns an edit form for a entity.
 *
 * Rendered entity use entity types machine name as subtype name.
 *
 * @see genero_hooks_entity_label_get_content_types()
 */
function genero_hooks_entity_label_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['markup'] = array(
    '#title' => t('Title tag'),
    '#type' => 'select',
    '#options' => array(
      'none' => t('- No tag -'),
      'h1' => t('h1'),
      'h2' => t('h2'),
      'h3' => t('h3'),
      'h4' => t('h4'),
      'h5' => t('h5'),
      'h6' => t('h6'),
      'div' => t('div'),
    ),
    '#default_value' => $conf['markup'],
  );

  $form['id'] = array(
    '#title' => t('CSS id to use'),
    '#type' => 'textfield',
    '#default_value' => $conf['id'],
  );

  $form['class'] = array(
    '#title' => t('CSS class to use'),
    '#type' => 'textfield',
    '#default_value' => $conf['class'],
  );

  $form['link'] = array(
    '#title' => t('Link to entity'),
    '#type' => 'checkbox',
    '#default_value' => $conf['link'],
    '#description' => t('Check here to make the name link to the entity page.'),
  );
  return $form;
}

/**
 * Submit handler for the custom type settings form.
 */
function genero_hooks_entity_label_content_type_edit_form_submit(&$form, &$form_state) {
  // Copy everything from our defaults.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Implements hook_PLUGIN_content_type_render().
 *
 * Ctools requires us to return a block.
 *
 * @see ctools_content_render()
 */
function genero_hooks_entity_label_content_type_render($entity_type, $conf, $panel_args, $context) {
  if ($context->empty) {
    return;
  }
  $entity_id = $context->argument;
  $entity = entity_load_single($entity_type, $entity_id);
  $entity_uri = entity_uri($entity_type, $entity);
  $entity_label = entity_label($entity_type, $entity);

  $content = !empty($conf['link']) ? l($entity_label, $entity_uri['path'], $entity_uri['options']) : check_plain($entity_label);

  // Build any surrounding markup if so configured
  if (isset($conf['markup']) && $conf['markup'] != 'none') {
    $markup = '<' . $conf['markup'];
    if (!empty($conf['id'])) {
      $markup .= ' id="' . $conf['id'] . '"';
    }
    if (!empty($conf['class'])) {
      $markup .= ' class="' . $conf['class'] . '"';
    }
    $markup .= '>' . $content . '</' . $conf['markup'] . '>' . "\n";
    $content = $markup;
  }

  $block = new stdClass();
  $block->module = 'entity';
  $block->title   = t('Label');
  $block->delta = $entity_type . '-label';
  $block->content = $content;

  return $block;
}

/**
 * Implements hook_PLUGIN_content_type_admin_title().
 *
 * Returns the administrative title for a type.
 */
function genero_hooks_entity_label_content_type_admin_title($entity_type, $conf, $contexts) {
  $entity_info = entity_get_info($entity_type);
  return t('@entity_type label"', array('@entity_type' => $entity_info['label']));
}
