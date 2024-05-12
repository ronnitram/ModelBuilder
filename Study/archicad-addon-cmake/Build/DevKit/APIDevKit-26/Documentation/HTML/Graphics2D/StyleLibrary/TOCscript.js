function TOCLayer (width, height)
{
	this.positionX = 0;
	this.positionY = 0;
	this.anchorPositionX = 0;
	this.anchorPositionY = 0;
	this.visible = false;
	this.layerWidth = width;
	this.layerHeight = height;

	this.setAnchorPosition = setAnchorPosition;
	this.setAnchorPositionX = setAnchorPositionX;
	this.setAnchorPositionY = setAnchorPositionY;
	this.getAnchorPositionX = getAnchorPositionX;
	this.getAnchorPositionY = getAnchorPositionY;
	this.setPosition = setPosition;
	this.setPositionX = setPositionX;
	this.setPositionY = setPositionY;
	this.getPositionX = getPositionX;
	this.getPositionY = getPositionY;
	this.getLayerWidth = getLayerWidth;
	this.getLayerHeight = getLayerHeight;
	this.isVisible = isVisible;
	this.moveOut = moveOut;
	this.moveIn = moveIn;

	var tocElement = document.getElementById('TOC');
	var tocBgElement = document.getElementById('TOC_bg');

	tocElement.style.width = width + 'px';
	tocElement.style.height = height + 'px';
	tocBgElement.style.width = width + 'px';
	tocBgElement.style.height = height + 'px';
	tocElement.style.visibility = 'hidden';
	tocBgElement.style.visibility = 'hidden';
}

function setAnchorPosition (x, y)
{
	this.anchorPositionX = x;
	this.anchorPositionY = y;
}

function setAnchorPositionX (x)
{
	this.anchorPositionX = x;
}

function setAnchorPositionY (y)
{
	this.anchorPositionY = y;
}

function getAnchorPositionX ()
{
	return this.anchorPositionX;
}

function getAnchorPositionY ()
{
	return this.anchorPositionY;
}

function setPosition (x, y)
{
	this.positionX = x;
	this.positionY = y;
	document.getElementById('TOC').style.left = x + 'px';
	document.getElementById('TOC').style.top = y + 'px';
	document.getElementById('TOC_bg').style.left = x + 'px';
	document.getElementById('TOC_bg').style.top = y + 'px';
}

function setPositionX (x)
{
	this.positionX = x;
	document.getElementById('TOC').style.left = x + 'px';
	document.getElementById('TOC_bg').style.left = x + 'px';
}

function setPositionY (y)
{
	this.positionY = y;
	document.getElementById('TOC').style.top = y + 'px';
	document.getElementById('TOC_bg').style.top = y + 'px';
}

function getPositionX ()
{
	return this.positionX;
}

function getPositionY ()
{
	return this.positionY;
}

function getLayerWidth ()
{
	return this.layerWidth;
}

function getLayerHeight ()
{
	return this.layerHeight;
}

function isVisible ()
{
	return this.visible;
}

function moveOut ()
{
	if (this.positionX > (this.anchorPositionX - this.layerWidth + 8)) {
		this.setPositionX (this.positionX - 10);
	} else {
		this.setPositionX (this.anchorPositionX - this.layerWidth - 2);
	}

	if (this. positionX > (this.anchorPositionX - this.layerWidth - 2)) {
		return true;
	} else {
		if (this.visible) {
			document.getElementById('TOC').style.visibility = 'hidden';
			document.getElementById('TOC_bg').style.visibility = 'hidden';
			this.visible = false;
		}
		return false;
	}
}

function moveIn ()
{
	if (!this.visible) {
		document.getElementById('TOC').style.visibility = 'visible';
		document.getElementById('TOC_bg').style.visibility = 'visible';
		this.visible = true;
	}

	if (this.positionX < (this.anchorPositionX - 10)) {
		this.setPositionX (this.positionX + 10);
	} else {
		this.setPositionX (this.anchorPositionX);
	}
	if (this. positionX < this.anchorPositionX) {
		return true;
	} else {
		return false;
	}
}


function findPos(obj)
{
	var curleft = 0;
	var curtop = 0;
	if (obj.offsetParent)
	{
		while (obj.offsetParent)
		{
			curtop += obj.offsetTop
			curleft += obj.offsetLeft
			obj = obj.offsetParent;
		}
	}
	else if (obj.x) {
		curleft += obj.x;
		curtop += obj.y;
	}
	return { x: curleft, y: curtop };
}

var tocLayer;

function setupTOCLayer ()
{
	if (tocLayer) {
		delete tocLayer;
	}
	contentPos = findPos(document.getElementById('content'));
	marginTop = contentPos.y - document.getElementById('docBegin').offsetHeight;
	tocLayer = new TOCLayer (250, 600);
	tocLayer.setAnchorPosition (-contentPos.x, -marginTop);
	tocLayer.setPosition (-contentPos.x - 250 - 2, -marginTop);
/*	document.getElementById('cover').style.width = (marginLeft - 2) + 'px';
	document.getElementById('cover').style.top = (marginTop) + 'px';*/
	document.getElementById('TOC_icon').style.left = (-contentPos.x) + 'px';
	document.getElementById('TOC_icon').style.top = (-marginTop) + 'px';
	document.getElementById('TOC_slider').src = ICONPATH + 'rightarrow.png';
}

var outTimer;
var inTimer;

function slideOut ()
{
	if (inTimer) {
		clearTimeout (inTimer);
	}
	if (tocLayer.moveOut ()) {
		outTimer = setTimeout("slideOut();", 10);
	}
}

function slideIn ()
{
	if (outTimer) {
		clearTimeout (outTimer);
	}
	if (tocLayer.moveIn ()) {
		inTimer = setTimeout("slideIn();", 10);
	}
}

function slideTOC (e)
{
	imageSrc = document.getElementById('TOC_slider').src;
	if (imageSrc.indexOf ('rightarrow.png') >= 0) {
		document.getElementById('TOC_slider').src = ICONPATH + 'leftarrow.png';
		slideIn ();
	} else {
		document.getElementById('TOC_slider').src = ICONPATH + 'rightarrow.png';
		slideOut ();
	}
}

function isCorrectBrowser ()
{
	locationURL = window.location.href;
	if ((locationURL.indexOf ("mk:@MSITStore") != 0) && (locationURL.indexOf ("ms-help://") != 0)) {
		return true;
	}
	return false;
}

function TOCTarget (targ)
{
	var toc = document.getElementById('TOC');
	var tocSlider = document.getElementById('TOC_slider');
	while (targ.parentNode) {
		if ((targ == toc) || (targ == tocSlider)) {
			return true;
		}
		targ = targ.parentNode;
	}
	return false;
}

function bodyClickHandler (e)
{
	var targ;
	if (!e) var e = window.event;
	if (e.target) targ = e.target;
	else if (e.srcElement) targ = e.srcElement;
	if (targ.nodeType == 3) // defeat Safari bug
		targ = targ.parentNode;

	if (!TOCTarget (targ)) {
		if (tocLayer.isVisible ()) {
			document.getElementById('TOC_slider').src = ICONPATH + 'rightarrow.png';
			slideOut ();
		}
	}
}

function insertTOC ()
{
	if (isCorrectBrowser ()) {
		document.write ("      <div style=\"position:relative; background-color:transparent; border:0px; padding:0px; z-index:9;\">\n");
		document.write ("        <div id=\"TOC_bg\" style=\"position:absolute; left:-40px; top:-4px; height:600px; width:250px; z-index:10; overflow:hidden; background-color:#FFFFFF; filter:alpha (opacity=90); border:1px none #000000; padding: 2px 2px 2px 2px;\"></div>\n");
		document.write ("        <div id=\"TOC\" style=\"position:absolute; left:-40px; top:-4px; height:600px; width:250px; z-index:11; overflow:auto; background-color:transparent; border:1px inset #000000; padding: 1px 1px 1px 1px;\">\n");
		document.write ("          <img src=\"" + ICONPATH + "../spacer.gif\" width=\"15px\" />\n");
		document.write ("          <span class=\"TreeviewSpanArea\">\n");
		document.write ("            <script>new tree (TREE_ITEMS, TREE_TPL);<\/script>\n");
/*		document.write ("            <noscript>\n");
		document.write ("              A tree to navigate the documentation will open here if you enable JavaScript in your browser.\n");
		document.write ("            </noscript>\n");*/
		document.write ("          </span>\n");
		document.write ("        </div>\n");
		/*document.write ("        <div id=\"cover\" style=\"position:absolute; left:-40px; top:-4px; height:602px; width:25px; z-index:12; overflow:hidden; background-color:#FFFFFF; border:1px none #000000; padding: 0px 0px 0px 0px;\"></div>\n");*/
		document.write ("        <div id=\"TOC_icon\" style=\"position:absolute; left:-40px; top:-4px; height:14px; width:14px; z-index:13; overflow:hidden; background-color:#FFFFFF; border:1px none #000000; padding: 0px 0px 0px 0px;\"><img id=\"TOC_slider\" src=\"" + ICONPATH + "rightarrow.png\"/></div>\n");
		document.write ("      </div>\n");
		var tocSlider = document.getElementById('TOC_slider');
		if (tocSlider.attachEvent) {
			tocSlider.attachEvent ('onclick', slideTOC);
			document.attachEvent ('onclick', bodyClickHandler);
		} else {
			tocSlider.addEventListener ('click', slideTOC, false);
			document.addEventListener ('click', bodyClickHandler, false);
		}
	}
}

function loadHandler ()
{
//	if ((navigator.OS == "win") && (navigator.org == "microsoft")) {
		BodyOnLoad ();
//	}
	if (isCorrectBrowser ()) {
		setupTOCLayer ();
	}
}

function resizeHandler ()
{
	BodyOnClick ();
	if (isCorrectBrowser ()) {
		setupTOCLayer ();
	}
}

window.onload = loadHandler;
window.onresize = resizeHandler;
