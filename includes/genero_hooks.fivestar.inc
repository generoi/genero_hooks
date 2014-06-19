<?php

/**
 * Implements hook_fivestar_widgets():
 */
function misc_hooks_fivestar_widgets() {
  $widgets = array(
    drupal_get_path('module', 'genero_hooks') . '/fivestar/fontawesome_stars.css' => 'fontawesome'
  );
  return $widgets;
}
