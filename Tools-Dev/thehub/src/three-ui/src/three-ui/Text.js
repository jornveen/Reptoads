var DisplayObject = require('./DisplayObject.js');


/**
 * Text
 * @extends DisplayObject
 *
 * Used internally by ThreeUI, shouldn't be used directly
 * Use ThreeUI.createText instead to create text
 *
 * @param {ThreeUI} ui
 * @param {string} text
 * @param {int} size
 * @param {string} font
 * @param {string} color
 * @param {int} x
 * @param {int} y
 */

var Text = function(ui, text, size, font, color, x, y) {
	this.textAlign = 'left';
	this.textBaseline = 'alphabetic';

	var type = "UIText";

	this.text = typeof text !== 'undefined' ? text : '';
	this.size = typeof size !== 'undefined' ? size : 12;
	this.font = typeof font !== 'undefined' ? font : 'Arial';
	this.color = typeof color !== 'undefined' ? color : '#000000';
	this.lineHeight = 1; // Modifier on text size

	var x = typeof x !== 'undefined' ? x : 0;
	var y = typeof y !== 'undefined' ? y : 0;

	// Run DisplayObject constructor on this object
	DisplayObject.bind(this)(ui, x, y, undefined, undefined, type);
};

Text.prototype = Object.create(DisplayObject.prototype);

/**
 * Draw this Text onto the provided context
 * Used internally by DisplayObject.render
 *
 * @param {CanvasRenderingContext2D} context
 * @param {int} x
 * @param {int} y
 */

Text.prototype.draw = function(context, x, y) {
	context.font = this.size + 'px ' + this.font;
	context.fillStyle = this.color;
	context.textAlign = this.textAlign;
	context.textBaseline = this.textBaseline;

	var lines = this.text.split('\n');

	var midLinePoint = 0; // Default to top
	if (this.textVerticalAlign === 'center') {
		midLinePoint = lines.length / 2 - 0.5;
	} else if (this.textVerticalAlign === 'bottom') {
		midLinePoint = lines.length - 1;
	}

	for (var idx = 0;idx < lines.length;idx++) {
		var line = lines[idx];
		var lineY = y + (idx - midLinePoint) * (this.size * this.lineHeight);
		console.log([x,lineY])
		context.fillText(line, x, lineY);
	}
};

    /**
     * Uses canvas.measureText to compute and return the width of the given text of given font in pixels.
     *
     * @param {String} text The text to be rendered.
     * @param {String} font The css font descriptor that text is to be rendered with (e.g. "bold 14px verdana").
     *
     * @see http://stackoverflow.com/questions/118241/calculate-text-width-with-javascript/21015393#21015393
     */
    Text.prototype.getMetrix = function (context,text){
		context.font = this.size + 'px ' + this.font;
		context.fillStyle = this.color;
		context.textAlign = this.textAlign;
		context.textBaseline = this.textBaseline;
		text = (text == undefined)? this.text:text;
        var metrics = context.measureText(text);
        return metrics;
    }
    Text.prototype.getTextWidth = function (context,text){
		return this.getMetrix(context,text).width;
    }

// Export Text as module
module.exports = Text;
