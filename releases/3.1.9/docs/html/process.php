<?php
   $command = array_shift($argv);
   $output = array_shift($argv);
   $file = array_shift($argv);
   $version = "3.1.9";
   $shorterVersion = "3.1";
   $package = "deal319";
   if ($output == 'site') {
     $bridgeFantasia = "../";
   } else {
     $bridgeFantasia = "http://bridge.thomasoandrews.com/";
   }
   $copyright = "1996-2010";
   function analytics() {
     global $output;
     if ($output == 'site') {
       include("analytics.htmlf");
     }
   }

   include($file);
