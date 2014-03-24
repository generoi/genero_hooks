<?php

/**
 * Implements hook_token_info().
 */
function genero_hooks_token_info() {
  $info['tokens']['site']['host'] = array(
    'name' => t('Host'),
    'description' => t('Sites front page without protocol and language prefix'),
  );
  return $info;
}

/**
 * Implements hook_tokens().
 */
function genero_hooks_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ($type == 'site') {
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'host':
          $front = url('<front>', array('absolute' => TRUE));
          $replacements[$original] = parse_url($front, PHP_URL_HOST);
          break;
      }
    }
  }
  return $replacements;
}
