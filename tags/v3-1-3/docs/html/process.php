<?php
   $command = array_shift($argv);
   $output = array_shift($argv);
   $file = array_shift($argv);
   $version = "3.1.3";
   $shorterVersion = "3.1";
   $package = "deal313";
   if ($output == 'site') {
     $bridgeFantasia = "../";
   } else {
     $bridgeFantasia = "http://bridge.thomasoandrews.com/";
   }
   include($file);
