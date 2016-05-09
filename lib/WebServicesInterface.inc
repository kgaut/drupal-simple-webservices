<?php

interface WebServicesInterface {
  public static function getDefinition();

  public static function checkAccess();
}