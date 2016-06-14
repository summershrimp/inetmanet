//
// Copyright (C) 2016 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//

#include <cstdlib>
#include "LinearGaugeFigure.h"

//TODO namespace inet { -- for the moment commented out, as OMNeT++ 5.0 cannot instantiate a figure from a namespace
using namespace inet;

Register_Class(LinearGaugeFigure);

#if OMNETPP_VERSION >= 0x500

static const double BORDER_WIDTH_PERCENT = 0.05;
static const double AXIS_WIDTH_PERCENT = 0.03;
static const double FRAME_PERCENT = 0.1;
static const double TICK_BIG_LENGTH_PERCENT = 0.17;
static const double TICK_SMALL_LENGTH_PERCENT = 0.27;
static double NUMBER_Y_PERCENT = 0.01;
static double NUMBER_FONTSIZE_PERCENT = 0.3;
static double NEEDLE_WIDTH_PERCENT = 0.05;
static double NEEDLE_HEIGHT_PERCENT = 0.6;

// Properties
static const char *PKEY_BACKGROUND_COLOR = "backgroundColor";
static const char *PKEY_NEEDLE_COLOR = "needleColor";
static const char *PKEY_CORNER_RADIUS = "cornerRadius";
static const char *PKEY_MIN_VALUE = "min";
static const char *PKEY_MAX_VALUE = "max";
static const char *PKEY_LABEL = "label";
static const char *PKEY_LABEL_FONT = "labelFont";
static const char *PKEY_LABEL_COLOR = "labelColor";
static const char *PKEY_TICK_SIZE = "tickSize";
static const char *PKEY_POS = "pos";
static const char *PKEY_SIZE = "size";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_BOUNDS = "bounds";

LinearGaugeFigure::LinearGaugeFigure(const char *name) : cGroupFigure(name)
{
    addChildren();
}

LinearGaugeFigure::~LinearGaugeFigure()
{
    // delete figures which is not in canvas
    for(int i = numTicks; i < tickFigures.size(); ++i)
    {
        delete tickFigures[i];
        delete numberFigures[i];
    }
}

cFigure::Rectangle LinearGaugeFigure::getBounds() const
{
    return backgroundFigure->getBounds();
}

void LinearGaugeFigure::setBounds(Rectangle rect)
{
    backgroundFigure->setBounds(rect);
    layout();
}

cFigure::Color LinearGaugeFigure::getBackgroundColor() const
{
    return backgroundFigure->getFillColor();
}

void LinearGaugeFigure::setBackgroundColor(cFigure::Color color)
{
    backgroundFigure->setFillColor(color);
}

cFigure::Color LinearGaugeFigure::getNeedleColor() const
{
    return needle->getLineColor();
}

void LinearGaugeFigure::setNeedleColor(cFigure::Color color)
{
    needle->setLineColor(color);
}

const char* LinearGaugeFigure::getLabel() const
{
    return labelFigure->getText();
}

void LinearGaugeFigure::setLabel(const char *text)
{
    labelFigure->setText(text);
}

cFigure::Font LinearGaugeFigure::getLabelFont() const
{
    return labelFigure->getFont();
}

void LinearGaugeFigure::setLabelFont(cFigure::Font font)
{
    labelFigure->setFont(font);
}

cFigure::Color LinearGaugeFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void LinearGaugeFigure::setLabelColor(cFigure::Color color)
{
    labelFigure->setColor(color);
}

double LinearGaugeFigure::getMin() const
{
    return min;
}

void LinearGaugeFigure::setMin(double value)
{
    if(min != value)
    {
        min = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getMax() const
{
    return max;
}

void LinearGaugeFigure::setMax(double value)
{
    if(max != value)
    {
        max = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getTickSize() const
{
    return tickSize;
}

void LinearGaugeFigure::setTickSize(double value)
{
    if(tickSize != value)
    {
        tickSize = value;
        redrawTicks();
        refresh();
    }
}

double LinearGaugeFigure::getCornerRadius() const
{
    return backgroundFigure->getCornerRx();
}

void LinearGaugeFigure::setCornerRadius(double radius)
{
    backgroundFigure->setCornerRadius(radius);
}

void LinearGaugeFigure::parse(cProperty *property)
{
    cGroupFigure::parse(property);
    setBounds(parseBounds(property));

    // Set default
    redrawTicks();

    const char *s;
    if ((s = property->getValue(PKEY_BACKGROUND_COLOR)) != nullptr)
        setBackgroundColor(parseColor(s));
    if ((s = property->getValue(PKEY_NEEDLE_COLOR)) != nullptr)
        setNeedleColor(parseColor(s));
    if ((s = property->getValue(PKEY_LABEL)) != nullptr)
        setLabel(s);
    if ((s = property->getValue(PKEY_LABEL_FONT)) != nullptr)
        setLabelFont(parseFont(s));
    if ((s = property->getValue(PKEY_LABEL_COLOR)) != nullptr)
        setLabelColor(parseColor(s));
    // This must be initialize before min and max because it is possible to be too much unnecessary tick and number
    if ((s = property->getValue(PKEY_TICK_SIZE)) != nullptr)
        setTickSize(atof(s));
    if ((s = property->getValue(PKEY_MIN_VALUE)) != nullptr)
        setMin(atof(s));
    if ((s = property->getValue(PKEY_MAX_VALUE)) != nullptr)
        setMax(atof(s));
    if ((s = property->getValue(PKEY_CORNER_RADIUS)) != nullptr)
        setCornerRadius(atof(s));
}

const char **LinearGaugeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = {PKEY_BACKGROUND_COLOR, PKEY_NEEDLE_COLOR, PKEY_LABEL, PKEY_LABEL_FONT,
                                   PKEY_LABEL_COLOR, PKEY_MIN_VALUE, PKEY_MAX_VALUE, PKEY_TICK_SIZE,
                                   PKEY_CORNER_RADIUS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS, nullptr};
        concatArrays(keys, cGroupFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void LinearGaugeFigure::addChildren()
{
    backgroundFigure = new cRectangleFigure("background");
    needle = new cLineFigure("needle");
    labelFigure = new cTextFigure("label");
    axisFigure = new cLineFigure("axis");

    backgroundFigure->setFilled(true);
    backgroundFigure->setFillColor(Color("#b8afa6"));

    needle->setLineColor("red");
    labelFigure->setAnchor(cFigure::ANCHOR_N);

    addFigure(backgroundFigure);
    addFigure(axisFigure);
    addFigure(needle);
    addFigure(labelFigure);
}

void LinearGaugeFigure::setValue(int series, simtime_t timestamp, double newValue)
{
    ASSERT(series == 0);
    // Note: we currently ignore timestamp
    if (value != newValue) {
        value = newValue;
        refresh();
    }
}

void LinearGaugeFigure::setTickGeometry(cLineFigure *tick, int index)
{
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth();
    double x = axisFigure->getStart().x + axisFigure->getLineWidth()/2 + index*axisWidth/(numTicks - 1);
    tick->setStart(Point(x, getBounds().getCenter().y));

    Point endPos = tick->getStart();
    endPos.y -= !(index % 3) ? getBounds().height * TICK_SMALL_LENGTH_PERCENT :
                               getBounds().height * TICK_BIG_LENGTH_PERCENT;
    tick->setEnd(endPos);
    tick->setLineWidth(getBounds().height * AXIS_WIDTH_PERCENT);
}

void LinearGaugeFigure::setNumberGeometry(cTextFigure *number, int index)
{
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth()/2;
    double x = axisFigure->getStart().x + axisFigure->getLineWidth()/2 + index*axisWidth/(numTicks - 1);
    Point textPos = Point(x, axisFigure->getStart().y + getBounds().height * NUMBER_Y_PERCENT);
    number->setPosition(textPos);
    number->setFont(cFigure::Font("", getBounds().height * NUMBER_FONTSIZE_PERCENT, 0));
}

void LinearGaugeFigure::setNeedleGeometry()
{
    needle->setLineWidth(getBounds().height * NEEDLE_WIDTH_PERCENT);

    double x = axisFigure->getStart().x + axisFigure->getLineWidth()/2;
    double axisWidth = axisFigure->getEnd().x - axisFigure->getStart().x - axisFigure->getLineWidth();
    if(std::isnan(value) || value < min)
        x -= needle->getLineWidth();
    else if(value > max)
        x = axisFigure->getEnd().x + needle->getLineWidth();
    else
        x += (value - min)*axisWidth/(max - min);

    needle->setStart(Point(x, axisFigure->getStart().y - getBounds().height * NEEDLE_HEIGHT_PERCENT/2));
    needle->setEnd(Point(x, axisFigure->getStart().y + getBounds().height * NEEDLE_HEIGHT_PERCENT/2));
}

void LinearGaugeFigure::redrawTicks()
{
    ASSERT(tickFigures.size() == numberFigures.size());

    int prevNumTicks = numTicks;
    numTicks = std::max(0.0, std::abs(max - min) / tickSize + 1);

    // Allocate ticks and numbers if needed
    if(numTicks > tickFigures.size())
        while(numTicks > tickFigures.size())
        {
            cLineFigure *tick = new cLineFigure();
            cTextFigure *number = new cTextFigure();

            number->setAnchor(cFigure::ANCHOR_N);

            tickFigures.push_back(tick);
            numberFigures.push_back(number);
        }

    // Add or remove figures from canvas according to previous number of ticks
    for(int i = numTicks; i < prevNumTicks; ++i)
    {
        removeFigure(tickFigures[i]);
        removeFigure(numberFigures[i]);
    }
    for(int i = prevNumTicks; i < numTicks; ++i)
    {
        addFigureBelow(tickFigures[i], needle);
        addFigureBelow(numberFigures[i], needle);
    }

    for(int i = 0; i < numTicks; ++i)
    {
        setTickGeometry(tickFigures[i], i);

        char buf[32];
        sprintf(buf, "%g", min + i*tickSize);
        numberFigures[i]->setText(buf);
        setNumberGeometry(numberFigures[i], i);
     }
}

void LinearGaugeFigure::layout()
{
    backgroundFigure->setLineWidth(getBounds().height * BORDER_WIDTH_PERCENT);

    double y = getBounds().getCenter().y;
    axisFigure->setLineWidth(getBounds().height * AXIS_WIDTH_PERCENT);
    axisFigure->setStart(Point(getBounds().x + getBounds().width*FRAME_PERCENT + axisFigure->getLineWidth()/2, y));
    axisFigure->setEnd(Point(getBounds().x + getBounds().width*(1-FRAME_PERCENT) + axisFigure->getLineWidth()/2, y));

    for(int i = 0; i < numTicks; ++i)
    {
        setTickGeometry(tickFigures[i], i);
        setNumberGeometry(numberFigures[i], i);
    }

    setNeedleGeometry();
    labelFigure->setPosition(Point(getBounds().getCenter().x, getBounds().y + getBounds().height));
}

void LinearGaugeFigure::refresh()
{
    setNeedleGeometry();
}

#endif // omnetpp 5

// } // namespace inet
