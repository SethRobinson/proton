//Every Proton .html file includes this file, it handles things like uploading and download, easier than changing all the 
//html files each time I want to make a change

 
document.writeln('<input type="file" style="display:none" id="uploader" onchange="previewFile()">');

//Filesaver stuff
 	/*! @source http://purl.eligrey.com/github/FileSaver.js/blob/master/FileSaver.js */
var saveAs=saveAs||function(e){"use strict";if("undefined"==typeof navigator||!/MSIE [1-9]\./.test(navigator.userAgent)){var t=e.document,n=function(){return e.URL||e.webkitURL||e},o=t.createElementNS("http://www.w3.org/1999/xhtml","a"),r="download"in o,i=function(e){var t=new MouseEvent("click");e.dispatchEvent(t)},a=/Version\/[\d\.]+.*Safari/.test(navigator.userAgent),c=e.webkitRequestFileSystem,d=e.requestFileSystem||c||e.mozRequestFileSystem,u=function(t){(e.setImmediate||e.setTimeout)(function(){throw t},0)},s="application/octet-stream",f=0,l=4e4,v=function(e){var t=function(){"string"==typeof e?n().revokeObjectURL(e):e.remove()};setTimeout(t,l)},p=function(e,t,n){t=[].concat(t);for(var o=t.length;o--;){var r=e["on"+t[o]];if("function"==typeof r)try{r.call(e,n||e)}catch(i){u(i)}}},w=function(e){return/^\s*(?:text\/\S*|application\/xml|\S*\/\S*\+xml)\s*;.*charset\s*=\s*utf-8/i.test(e.type)?new Blob(["\uFEFF",e],{type:e.type}):e},y=function(t,u,l){l||(t=w(t));var y,m,S,h=this,R=t.type,O=!1,g=function(){p(h,"writestart progress write writeend".split(" "))},b=function(){if(m&&a&&"undefined"!=typeof FileReader){var o=new FileReader;return o.onloadend=function(){var e=o.result;m.location.href="data:attachment/file"+e.slice(e.search(/[,;]/)),h.readyState=h.DONE,g()},o.readAsDataURL(t),void(h.readyState=h.INIT)}if((O||!y)&&(y=n().createObjectURL(t)),m)m.location.href=y;else{var r=e.open(y,"_blank");void 0===r&&a&&(e.location.href=y)}h.readyState=h.DONE,g(),v(y)},E=function(e){return function(){return h.readyState!==h.DONE?e.apply(this,arguments):void 0}},N={create:!0,exclusive:!1};return h.readyState=h.INIT,u||(u="download"),r?(y=n().createObjectURL(t),void setTimeout(function(){o.href=y,o.download=u,i(o),g(),v(y),h.readyState=h.DONE})):(e.chrome&&R&&R!==s&&(S=t.slice||t.webkitSlice,t=S.call(t,0,t.size,s),O=!0),c&&"download"!==u&&(u+=".download"),(R===s||c)&&(m=e),d?(f+=t.size,void d(e.TEMPORARY,f,E(function(e){e.root.getDirectory("saved",N,E(function(e){var n=function(){e.getFile(u,N,E(function(e){e.createWriter(E(function(n){n.onwriteend=function(t){m.location.href=e.toURL(),h.readyState=h.DONE,p(h,"writeend",t),v(e)},n.onerror=function(){var e=n.error;e.code!==e.ABORT_ERR&&b()},"writestart progress write abort".split(" ").forEach(function(e){n["on"+e]=h["on"+e]}),n.write(t),h.abort=function(){n.abort(),h.readyState=h.DONE},h.readyState=h.WRITING}),b)}),b)};e.getFile(u,{create:!1},E(function(e){e.remove(),n()}),E(function(e){e.code===e.NOT_FOUND_ERR?n():b()}))}),b)}),b)):void b())},m=y.prototype,S=function(e,t,n){return new y(e,t,n)};return"undefined"!=typeof navigator&&navigator.msSaveOrOpenBlob?function(e,t,n){return n||(e=w(e)),navigator.msSaveOrOpenBlob(e,t||"download")}:(m.abort=function(){var e=this;e.readyState=e.DONE,p(e,"abort")},m.readyState=m.INIT=0,m.WRITING=1,m.DONE=2,m.error=m.onwritestart=m.onprogress=m.onwrite=m.onabort=m.onerror=m.onwriteend=null,S)}}("undefined"!=typeof self&&self||"undefined"!=typeof window&&window||this.content);"undefined"!=typeof module&&module.exports?module.exports.saveAs=saveAs:"undefined"!=typeof define&&null!==define&&null!==define.amd&&define([],function(){return saveAs});
 
//File handling stuff
	
 function previewFile() 
 {
  var file    = document.querySelector('input[type=file]').files[0];
  var reader  = new FileReader();
   
  reader.addEventListener("load", function () 
  {
		//    preview.src = reader.result;
		//alert('Got file');
  }, false);
 
	reader.onloadend = (e) => 
	{ 
		
		var fileName = document.querySelector('input[type=file]').files[0].name;
		
    //let base64String = reader.result.split(',').pop();
     console.log('Got uploaded file '+fileName+', processing it');
    
    // Create a pointer using the 'Glue' method and the String value
    
    var data = new Uint8Array(reader.result);

		var outFileName = "proton_temp.tmp"; //we use a temp file because we don't actually want to overwrite something we need
	
		
		try
		{
				Module['FS_unlink']('/'+outFileName);
	  } catch(err) 
	  {
			 console.log('FS_unlink: '+err);
		}
		
    Module['FS_createDataFile']('/', outFileName, data, true, true, true);
    document.querySelector('input[type=file]').value = '';
 
    //var ptr = allocate(intArrayFromString(outFileName), 'i8', ALLOC_NORMAL); //not needed I guess
    //void_systemMessage = Module.cwrap('PROTON_SystemMessage', null, ['number', 'number', 'string'])
    //void_systemMessage(53,0, outFileName);
   
     void_guiMessage = Module.cwrap('PROTON_GUIMessage', null, ['number', 'number', 'string'])
     void_guiMessage(53,0, fileName);
    //53 is MESSAGE_TYPE_HTML5_GOT_UPLOAD
 };
 
  if (file)
   {
   	 reader.readAsArrayBuffer(file);
  }
}

//stuff for file saving
  function saveFileFromMemoryFSToDisk(memoryFSname,localFSname)     // This can be called by C++ code
  {
     var data=FS.readFile(memoryFSname);
     var blob;
     var isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
     if(isSafari) {
        blob = new Blob([data.buffer], {type: "application/octet-stream"});
     } else {
        blob = new Blob([data.buffer], {type: "application/octet-binary"});
     }
     saveAs(blob, localFSname);
  }
  
  
   	var textNum = 0;
  
  //Uncomment below to disable webassembly loading
  //UnityLoader.SystemInfo.hasWasm = false;

        
        var Module = 
        {
        	
        	setStatus: function(text) 
  	{
        	
        	
      //console.log('SetStatus: ' + text) 
      const progress = document.querySelector("#loader .progress");
     
      if (!Module.progress)
      {
      	//set it up
      progress.style.display = "block";
      Module.progress = progress.querySelector(".full");
      loader.querySelector(".spinner").style.display = "none";
      Module.progress.style.transform = `scaleX(${0})`;
       } else
     	{
				//continue it     		
	 				var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
	 				if (m) 
	 					{
	 						loader.querySelector(".loadingtext").innerHTML = "Loading stuff...";
		 					 var cur = parseInt(m[2])*100;
		           var progressMax = parseInt(m[4])*100;
		           //console.log('cur: ' + cur+' max: '+progressMax) ;
		           Module.progress.style.transform = `scaleX(${cur/progressMax})`;
		        } else
		       	{
	         		//done loading
	         		 UpdateText(); //will show index 0
	     			   loader.style.display = "none";
	     			   loader.querySelector(".spinner").style.display = "block";
	    			}
      	}
     	},
        	  print: function(text) {console.log('>> ' + text) },
  					printErr: function(text) { console.log(appName+' ERROR: ' + text) },
        	
            onRuntimeInitialized: function() 
            {
              
            loader.querySelector(".spinner").style.display = "none";
            loader.style.display = "none";
            Module.ccall('mainf', null, null);
                
            }, 
            canvas: (function() 
            {
                var canvas = document.getElementById('canvas');
                return canvas;
                })()
        };

        var start_function = function(o) 
        {
            loader.querySelector(".spinner").style.display = "none";
            loader.style.display = "none";
            Module.ccall('mainf', null, null);
        };


          (function() 
          {
            var memoryInitializer = appName+'.js.mem';
            if (typeof Module['locateFile'] === 'function') {
              memoryInitializer = Module['locateFile'](memoryInitializer);
            } else if (Module['memoryInitializerPrefixURL']) {
              memoryInitializer = Module['memoryInitializerPrefixURL'] + memoryInitializer;
            }
            var xhr = Module['memoryInitializerRequest'] = new XMLHttpRequest();
            xhr.open('GET', memoryInitializer, true);
            xhr.responseType = 'arraybuffer';
            xhr.send(null);
          })();

          var script = document.createElement('script');
          script.src = appName+".js";
          document.body.appendChild(script);


  var textIntervalHandle;
  var textTimerIntervalMS = 5000; //how fast the text updates after the initial two
  
  var textCommentArray = [
   "This can really take a while.",
   "Webgl apps like this are still relatively new, they will get faster to initialize in time.",
   "While slow to initialize, this is safer to run and always up to date as compared to a downloadable that you would need to install.",
   "Make sure you're using a 64 bit browser.  Chrome or Firefox are good choices.",
   "Did you know RTsoft has been making weird games since 1989?  It's true, Seth is just that old.",
   "It's looking like you'll be old too before this game starts.",
   "You've gotta be bored out of your gourd by now.  Sorry about the wait.",
   "This doesn't work great on mobile.  Chrome on Android *might* work, if you wait long enough.",
   "It's possible this could take like 5 minutes.  Wait if you want, or go try it on a powerful desktop machine maybe."
   ];
  
  
	function UpdateText() 
	{
   if (textCommentArray.length > textNum)
   {
   	  //we have more to show
      const loader = document.querySelector("#loader");
  
     	loader.querySelector(".loadingtext").innerHTML = textCommentArray[textNum];
    
      textNum++;
   
	    if (textNum > 1)
	    {
	    	//keep rescheduling them after the first two messages
	     textIntervalHandle = window.setTimeout(UpdateText, textTimerIntervalMS);
	    }
    }
	}

