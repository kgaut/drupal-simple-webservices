<?php

abstract class SimpleEntityWebServices extends SimpleWebServices {
  protected static $entity_type;
  protected static $entity_bundle;
  protected static $content_type_path;
  protected static $content_type_label;
  protected static $has_retrieve = TRUE;
  protected static $results_keys_to_keep = ['nid','title','created','changed','uri'];


  public static function getDefinition() {

    $definition = [
      'operations' => []
    ];

    $definition['operations']['index'] = [
      'callback' => get_called_class().'::getEntitiesList',
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
    ];

    if(static::$has_retrieve) {
      $definition['operations']['retrieve'] = [
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
      ];
    }
    return $definition;
  }

  public static function getEntitiesList($page, $order, $direction, $created_after, $changed_after, $fields, $parameters, $page_size) {
    static::checkAccess();
    $query = static::getEntitiesListQuery($page, $order, $direction, $created_after, $changed_after, $fields, $parameters, $page_size);

    $results = services_resource_execute_index_query($query);
    $results = static::filterResults($results);
    return $results;
  }

  public static function getEntitiesListQuery($page, $order, $direction, $created_after, $changed_after, $fields, $parameters, $page_size) {
    $query = new EntityFieldQuery();

    $query->entityCondition('entity_type', static::$entity_type)
      ->entityCondition('bundle', static::$entity_bundle)
      ->propertyOrderBy($order,$direction)
      ->range($page * $page_size, $page_size);

    if (isset($parameters) && is_array($parameters)) {
      foreach ($parameters as $parameter => $parameter_value) {
        $query->condition($parameter, services_str_getcsv($parameter_value), 'IN');
      }
    }

    if($created_after && is_numeric($created_after)) {
      $query->condition('created',$created_after,'>');
    }
    if($changed_after && is_numeric($changed_after)) {
      $query->condition('changed',$changed_after,'>');
    }
    return $query;
  }

  protected static function filterResults($results) {
    $results_filtred = [];
    $endpoint_name = services_get_server_info('endpoint');
    $endpoint = services_endpoint_load($endpoint_name);
    foreach($results[static::$entity_bundle] as $entity) {
      $entity = entity_load(static::$entity_bundle,array($entity->id));
      $entity = array_pop($entity);
      if(static::$has_retrieve) {
        $entity->uri = url('/'.$endpoint->path.'/'.static::$content_type_path.'/'.$entity->id, array('absolute' => TRUE));
      }
      $results_filtred[] = $entity;
    }
    return $results_filtred;
  }
}