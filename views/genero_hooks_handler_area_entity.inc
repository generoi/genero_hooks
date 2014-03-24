<?php
/**
 * @file
 * Renders a full entity in a views area.
 *
 * Patched version of entity modules `entity_views_handler_area_entity` to
 * allow contextual filter arguments.
 *
 * @see https://drupal.org/files/issues/entity_id_token-1957266-12.patch
 */

class genero_hooks_handler_area_entity extends entity_views_handler_area_entity {
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['entity_id']['#description'] = t('Choose the entity you want to display in the area. To render an entity given by a contextual filter use "%1"');
    return $form;
  }

  /**
   * Render an entity using the view mode.
   */
  public function render_entity($entity_type, $entity_id, $view_mode) {
    if (!empty($entity_type) && !empty($entity_id) && !empty($view_mode)) {
      $tokens = $this->get_render_tokens();
      // Replace argument tokens in entity id.
      $entity_id = strtr($entity_id, $tokens);

      if (!is_numeric($entity_id)) {
        return '';
      }

      $entity = entity_load_single($entity_type, $entity_id);
      if (!empty($this->options['bypass_access']) || entity_access('view', $entity_type, $entity)) {
        $render = entity_view($entity_type, array($entity), $view_mode);
        $render_entity = reset($render);
        return drupal_render($render_entity);
      }
    }
    else {
      return '';
    }
  }

  /**
   * Get the 'render' tokens to use for advanced rendering.
   *
   * This runs through all of the fields and arguments that
   * are available and gets their values. This will then be
   * used in one giant str_replace().
   */
  function get_render_tokens() {
    $tokens = array();
    if (!empty($this->view->build_info['substitutions'])) {
      $tokens = $this->view->build_info['substitutions'];
    }
    $count = 0;
    foreach ($this->view->display_handler->get_handlers('argument') as $arg => $handler) {
      $token = '%' . ++$count;
      if (!isset($tokens[$token])) {
        $tokens[$token] = '';
      }

      // Use strip tags as there should never be HTML in the path.
      // However, we need to preserve special characters like " that
      // were removed by check_plain().
      $tokens['%' . $count] = isset($this->view->args[$count - 1]) ? strip_tags(decode_entities($this->view->args[$count - 1])) : '';
    }

    return $tokens;
  }
}
