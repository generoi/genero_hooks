<?php

/**
 * Implements hook_token_info().
 */
function genero_hooks_token_info() {
  $info['tokens']['site']['host'] = array(
    'name' => t('Host'),
    'description' => t('Sites front page without protocol and language prefix'),
  );
  $info['tokens']['node']['media'] = array(
    'name' => t('Node media'),
    'descriptions' => t('Media from node'),
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
  if ($type == 'node' && !empty($data['node'])) {
    $node = $data['node'];
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'media':
          if ($body = field_get_items('node', $node, 'body')) {
            $value = $body[0]['value'];
            genero_hooks_media_filter($value);
            $replacements[$original] = $value;
          }
          break;
      }
    }
  }
  return $replacements;
}

/**
 * Extract a media url from a text.
 */
function genero_hooks_media_filter(&$value) {
  preg_match(MEDIA_WYSIWYG_TOKEN_REGEX, $value, $matches);
  $value = NULL;
  foreach ($matches as $key => $match) {
    $match = str_replace("[[", "", $match);
    $match = str_replace("]]", "", $match);
    $tag = $match;
    $tag_info = drupal_json_decode($tag);
    if ($file = file_load($tag_info['fid'])) {
      $value = file_create_url($file->uri);
    }
  }
}
