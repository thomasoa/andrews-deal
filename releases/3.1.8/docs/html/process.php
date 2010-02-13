<?php
   $command = array_shift($argv);
   $output = array_shift($argv);
   $file = array_shift($argv);
   $version = "3.1.8";
   $shorterVersion = "3.1";
   $package = "deal318";
   if ($output == 'site') {
     $bridgeFantasia = "../";
   } else {
     $bridgeFantasia = "http://bridge.thomasoandrews.com/";
   }
   $copyright = "1996-2008";
   function analytics() {
     global $output;
     if ($output == 'site') {
       include("analytics.htmlf");
     }
   }

   include($file);
