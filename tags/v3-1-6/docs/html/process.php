<?php
   $command = array_shift($argv);
   $output = array_shift($argv);
   $file = array_shift($argv);
   $version = "3.1.6";
   $shorterVersion = "3.1";
   $package = "deal316";
   if ($output == 'site') {
     $bridgeFantasia = "../";
   } else {
     $bridgeFantasia = "http://bridge.thomasoandrews.com/";
   }
   $copyright = "1996-2008";
   include($file);
