<?php

abstract class SimpleNodeWebServices extends SimpleWebServices {
  protected static $content_type;
  protected static $content_type_label;

  public static function getDefinition() {
    return [
      'operations' => [
        'index' => [
          'callback' => get_called_class().'::getNodeList',
          'args' => array(
            array(
              'name' => 'page',
              'optional' => TRUE,
              'type' => 'int',
              'description' => 'The zero-based index of the page to get, defaults to 0.',
              'default value' => 0,
              'source' => array('param' => 'page'),
            ),
            array(
              'name' => 'fields',
              'optional' => TRUE,
              'type' => 'string',
              'description' => 'The fields to get.',
              'default value' => '*',
              'source' => array('param' => 'fields'),
            ),
            array(
              'name' => 'parameters',
              'optional' => TRUE,
              'type' => 'array',
              'description' => 'Parameters array',
              'default value' => array(),
              'source' => array('param' => 'parameters'),
            ),
            array(
              'name' => 'pagesize',
              'optional' => TRUE,
              'type' => 'int',
              'description' => 'Number of records to get per page.',
              'default value' => variable_get('services_node_index_page_size', 20),
              'source' => array('param' => 'pagesize'),
            ),
          ),
          'access arguments' => array('access content'),
        ]
      ]
    ];
  }

  public static function getNodeList($page, $fields, $parameters, $page_size) {
    $node_select = db_select('node', 't')
      ->addTag('node_access')
      ->orderBy('sticky', 'DESC')
      ->orderBy('created', 'DESC');

    services_resource_build_index_query($node_select, $page, $fields, $parameters, $page_size, 'node');

    if (!user_access('administer nodes')) {
      $node_select->condition('status', 1);
    }
    dd(static::$content_type);
    $node_select->condition('type',static::$content_type);

    $results = services_resource_execute_index_query($node_select);

    return services_resource_build_index_list($results, 'node', 'nid');
  }
}