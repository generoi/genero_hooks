<?php

/**
 * Implements hook_preprocess_panels_ipe_pane_wrapper().
 */
function genero_hooks_preprocess_panels_ipe_pane_wrapper(&$vars) {
  $pane = $vars['pane'];
  $display = $vars['display'];
  $renderer = $vars['renderer'];

  if (user_access('administer advanced pane settings')) {
    $css_link = array(
      'title' => t('CSS'),
      'href' => $renderer->get_url('pane-css', $pane->pid),
      'attributes' => array(
        'class' => array('ctools-use-modal', 'pane-css'),
        'title' => t('CSS'),
      ),
    );

    // Make sure css is added just after style.
    $style_link_key = array_search('style', array_keys($vars['links']));
    $links_start = array_slice($vars['links'], 0, $style_link_key + 1);
    $links_end = array_slice($vars['links'], $style_link_key);

    // Add css link
    $links_start['css'] = $css_link;
    $vars['links'] = array_merge($links_start, $links_end);
  }
}

/**
 * Implements hook_hooks_form_FORM_ID_alter().
 *
 * Add custom configuration elements  to views content panes.
 */
function genero_hooks_form_views_content_views_panes_content_type_edit_form_alter(&$form, &$form_state, $form_id) {
  $conf = $form_state['conf'];
  list($name, $display_id) = explode('-', $form_state['subtype_name']);
  $view = views_get_view($name);
  if (empty($view)) {
    return;
  }
  $view->set_display($display_id);
  // Make the more text configurable.
  $form['exposed']['more_text'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($conf['more_text']) ? $conf['more_text'] : $view->display_handler->use_more_text(),
    '#title' => t('More link text'),
    '#dependency' => array('more-link' => array(1)),
    '#weight' => 99,
  );
  $form['#submit'][] = 'genero_hooks_views_content_views_panes_content_type_edit_form_submit';
}

/**
 * Custom form callback storing custom views pane form element values.
 */
function genero_hooks_views_content_views_panes_content_type_edit_form_submit(&$form, &$form_state) {
  foreach (array('more_text') as $key) {
    if (isset($form_state['values']['exposed'][$key])) {
      $form_state['conf'][$key] = $form_state['values']['exposed'][$key];
    }
  }
}

/**
 * Implements hook_views_pre_view().
 */
function genero_hooks_views_pre_view(&$view) {
  if (isset($view->display_handler->options['pane_conf'])) {
    $conf = $view->display_handler->options['pane_conf'];
    // If the pane has set a custom more text, set it for the view before
    // rendering.
    if (!empty($conf['more_text'])) {
      $view->display_handler->set_option('use_more_text', t($conf['more_text']));
    }
  }
}
