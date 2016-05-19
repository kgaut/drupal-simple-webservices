<?php

abstract class SimpleNodeWebServices extends SimpleWebServices {
  protected static $content_type;
  protected static $content_type_path;
  protected static $content_type_label;
  protected static $results_keys_to_keep = ['nid','title','created','changed','uri'];
  protected static $result_details_keys_to_remove = ['rdf_mapping','data','tnid','translate','promote','sticky','comment','log','print_pdf_size','print_pdf_orientation'];


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
              'name' => 'order',
              'optional' => TRUE,
              'type' => 'string',
              'description' => 'The fieldname to order on',
              'default value' => 'created',
              'source' => array('param' => 'order'),
            ),
            array(
              'name' => 'direction',
              'optional' => TRUE,
              'type' => 'string',
              'description' => 'The direction for ordering',
              'default value' => 'DESC',
              'source' => array('param' => 'direction'),
            ),
            array(
              'name' => 'created_after',
              'optional' => TRUE,
              'type' => 'int',
              'description' => 'Content created after given timestamp',
              'default value' => null,
              'source' => array('param' => 'created_after'),
            ),
            array(
              'name' => 'changed_after',
              'optional' => TRUE,
              'type' => 'int',
              'description' => 'Content created/updated after given timestamp',
              'default value' => null,
              'source' => array('param' => 'changed_after'),
            ),
            array(
              'name' => 'pagesize',
              'optional' => TRUE,
              'type' => 'int',
              'description' => 'Number of records to get per page.',
              'default value' => 20,
              'source' => array('param' => 'pagesize'),
            ),
          ),
          'access arguments' => array('access content'),
        ],
        'retrieve' => [
          'callback' => get_called_class().'::getNodeDetails',
          'args' => [
            [
              'name' => 'nid',
              'optional' => FALSE,
              'source' => ['path' => 0],
              'type' => 'int',
              'description' => 'The nid of the node to get',
            ],
          ],
          'access callback' => '_node_resource_access',
          'access arguments' => ['view'],
          'access arguments append' => TRUE,
        ]
      ]
    ];
  }

  public static function getNodeList($page, $order, $direction, $created_after, $changed_after,  $page_size) {
    static::checkAccess();
    $query = static::getNodeListQuery($page, $order, $direction, $created_after, $changed_after, $page_size);

    $results = services_resource_execute_index_query($query);
    $results = static::filterResults($results);
    return $results;
  }

  public static function getNodeListQuery($page, $order, $direction, $created_after, $changed_after, $page_size) {
    $node_select = db_select('node', 't')
      ->addTag('node_access')
      ->orderBy($order, $direction);$node_select->range($page * $page_size, $page_size);
    $node_select->fields('t');


    if (!user_access('administer nodes')) {
      $node_select->condition('status', 1);
    }
    if($created_after && is_numeric($created_after)) {
      $node_select->condition('created',$created_after,'>');
    }
    if($changed_after && is_numeric($changed_after)) {
      $node_select->condition('changed',$changed_after,'>');
    }

    $node_select->condition('type',static::$content_type);
    return $node_select;
  }

  public static function getNodeDetails($nid) {
    static::checkAccess();
    $node = node_load($nid);

    if ($node) {
      $uri = entity_uri('node', $node);
      $node->path = url($uri['path'], array('absolute' => TRUE));
      unset($node->uri);
    }
    foreach (static::$result_details_keys_to_remove as $field_to_remove) {
      if(isset($node->{$field_to_remove})) {
        unset($node->{$field_to_remove});
      }
    }

    //Lets check field_permissions
    $node = services_field_permissions_clean('view', 'node', $node);
    return $node;
  }

  protected static function filterResults($results) {
    $results_filtred = [];
    $endpoint_name = services_get_server_info('endpoint');
    $endpoint = services_endpoint_load($endpoint_name);
    foreach ($results as $result) {
      $result_filtred = new stdClass();
      foreach ($result as $key => $value) {
        if(in_array($key,static::$results_keys_to_keep)) {
          $result_filtred->{$key} = $value;
        }
      }
      $result_filtred->uri = url('/'.$endpoint->path.'/'.static::$content_type_path.'/'.$result_filtred->nid, array('absolute' => TRUE));
      $results_filtred[] = $result_filtred;
    }
    return $results_filtred;
  }
}