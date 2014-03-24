<?php

/**
 * Implements hook_views_data().
 */
function genero_hooks_views_data() {
  // Provide our own entity area handler that accepts contextual filter
  // arguments.
  $data['entity__global']['entity_arguments'] = array(
    'title' => t('Rendered entity (arguments)'),
    'help' => t('Displays a single chosen entity and select the entity id using arguments from contextual filters.'),
    'area' => array(
      'handler' => 'genero_hooks_handler_area_entity',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_query_substitutions().
 */
function genero_hooks_views_query_substitutions() {
  // TODO: Chache this thing, so we don't have reload the view if it is already
  // loaded. Alternatively, make sure that Views does this alreday (which is
  // likely).
  $view = views_get_current_view();

  // Build the array of replacement patterns, but only if there is any arguments
  // set.
  $replacements = array();
  if (is_array($view->args)) {
    foreach($view->args as $index => $argument) {
      // Shift all indeces up one step, to sync with the outward-facing
      // denumeration used in the Views interface.
      $newindex = $index + 1;
      $replacements["!$newindex"] = $argument;
      // TODO: Use $view->argument to load the validated title for the argument
      // object, if there is any. For now, we only use the raw input value.
      $replacements["%$newindex"] = $argument;
    }
  }

  return $replacements;
}
