<?php

abstract class SimpleWebServices implements WebServicesInterface {

  public static function checkAccess() {
    return true;
  }
}