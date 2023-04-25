// here you write JS "handlers"
mergeInto(LibraryManager.library, 
{

JLIB_EnterString: function(message, defaultText)
{
        var pMessage = UTF8ToString(message);
        var pURL = UTF8ToString(defaultText);
	var person = prompt(pMessage, pURL);
	
	if (person == null) return 0;
	
	 console.log("Got "+person);
	 
	 // Create a pointer using the 'Glue' method and the String value
        var ptr = allocateUTF8(person);
    return ptr;
},


JLIB_GetURL: function ()
{
    //console.log("Querystring: "+decodeURIComponent(window.location.href));
    var ptr = allocateUTF8(decodeURIComponent(window.location.href));
    return ptr;
},

JLIB_OpenURL: function (URLStr)
  {
    var url = UTF8ToString(URLStr);
  
    var OpenPopup = function() 
    {
      //run the thing we're supposed to
      console.log("Doing OpenPopUp");
	
      window.open(url, "_blank");
      //unhook this, we're done
      var el = document.getElementById('canvas');
      if (el)
      {
	    el.removeEventListener('click', OpenPopup)
	    el.removeEventListener('touchend', OpenPopup)
	  } 
    };
    
    var el = document.getElementById('canvas');
    if (el)
    {
	    el.addEventListener('click', OpenPopup, false);
	    el.addEventListener('touchend', OpenPopup, false);
	  } else
	  {
	     console.log("JLIB_OpenURL: Can't find a canvas element. Is it called #canvas instead or something?  Check SharedJSLIB.js.  Calling without popup safety.");
	     window.open(url);
	  }
   
  },
  
  JLIB_OnClickSomethingByID: function (IDStr)
  {
    var idName = UTF8ToString(IDStr);
  
    var OpenPopup = function() 
    {
      //run the thing we're supposed to
      console.log("Doing JLIB_OnClickSomethingByID");
	
       document.getElementById(idName).click();
      //unhook this, we're done
      var el = document.getElementById('canvas');
      if (el)
      {
	    el.removeEventListener('click', OpenPopup)
	    el.removeEventListener('touchend', OpenPopup)
	  } 
    };
     
    var el = document.getElementById('canvas');
    if (el)
    {
    	 console.log("LIB_OnClickSomethingByID hooking into click");
	
	    el.addEventListener('click', OpenPopup, false);
	    el.addEventListener('touchend', OpenPopup, false);
	  } else
	  {
	     console.log("JLIB_OnClickSomethingByID: Can't find a canvas element. Is it called #canvas instead or something?  Check SharedJSLIB.js.  Calling without popup safety.");
	     document.getElementById(idName).click();
	  }
   
  },
});