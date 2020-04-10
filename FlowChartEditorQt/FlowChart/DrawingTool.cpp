#include "DrawingTool.h"

#include "DrawingPaper.h"
#include "../FlowChartEditor.h"

#include "FlowChart.h"
#include "Shape.h"
#include "SelectingTool.h"

#include "ScrollController.h"
#include "Scroll.h"

#include "MemoryController.h"

#include "Zoom.h"
#include "ZoomVisitor.h"
#include "CoordinateConverter.h"

#include "../GObject/Painter.h"

DrawingTool* DrawingTool::instance = 0;

DrawingTool::DrawingTool() {
}

DrawingTool* DrawingTool::Instance() {
	if (instance == 0) {
		instance = new DrawingTool;
	}
	return instance;
}

void DrawingTool::Destroy() {
	if (instance != 0) {
		delete instance;
		instance = 0;
	}
}

void DrawingTool::OnLButtonDown(DrawingPaper *canvas, QPoint point) {
	canvas->flowChart->UnSelectAll();

	Long positionX = 0;
	Long positionY = 0;
	if (canvas->scrollController != NULL) {
		positionX = canvas->scrollController->GetScroll(1)->value();
		positionY = canvas->scrollController->GetScroll(0)->value();
	}

	canvas->startX = point.x() + positionX;
	canvas->startY = point.y() + positionY;

	canvas->currentX = point.x() + positionX;
	canvas->currentY = point.y() + positionY;

	NShape *holdA4Paper = canvas->a4Paper->Clone();
	FlowChartVisitor *zoomVisitor = new ZoomVisitor(canvas->zoom);
	holdA4Paper->Accept(zoomVisitor);

	Long quotient;
	Long remainder;

	QPoint startReal(canvas->startX, canvas->startY);
	QPoint startVirtual = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertVirtual(startReal);

	quotient = startVirtual.x() * 100 / canvas->zoom->GetRate();
	remainder = startVirtual.x() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	startVirtual.setX(quotient);

	quotient = startVirtual.y() * 100 / canvas->zoom->GetRate();
	remainder = startVirtual.y() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	startVirtual.setY(quotient);

	startReal = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertReal(startVirtual);

	QPoint currentReal(canvas->currentX, canvas->currentY);
	QPoint currentVirtual = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertVirtual(currentReal);

	quotient = currentVirtual.x() * 100 / canvas->zoom->GetRate();
	remainder = currentVirtual.x() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	currentVirtual.setX(quotient);

	quotient = currentVirtual.y() * 100 / canvas->zoom->GetRate();
	remainder = currentVirtual.y() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	currentVirtual.setY(quotient);

	currentReal = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertReal(currentVirtual);

	canvas->startX = startReal.x();
	canvas->startY = startReal.y();

	canvas->currentX = currentReal.x();
	canvas->currentY = currentReal.y();

	canvas->templateSelected->Move(canvas->startX, canvas->startY);
	canvas->templateSelected->Select(true);
}

void DrawingTool::OnMouseMove(DrawingPaper *canvas, QPoint point) {
	Long positionX = 0;
	Long positionY = 0;
	if (canvas->scrollController != NULL) {
		positionX = canvas->scrollController->GetScroll(1)->value();
		positionY = canvas->scrollController->GetScroll(0)->value();
	}

	canvas->currentX = point.x() + positionX;
	canvas->currentY = point.y() + positionY;

	NShape *holdA4Paper = canvas->a4Paper->Clone();
	FlowChartVisitor *zoomVisitor = new ZoomVisitor(canvas->zoom);
	holdA4Paper->Accept(zoomVisitor);

	Long quotient;
	Long remainder;

	QPoint currentReal(canvas->currentX, canvas->currentY);
	QPoint currentVirtual = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertVirtual(currentReal);

	quotient = currentVirtual.x() * 100 / canvas->zoom->GetRate();
	remainder = currentVirtual.x() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	currentVirtual.setX(quotient);

	quotient = currentVirtual.y() * 100 / canvas->zoom->GetRate();
	remainder = currentVirtual.y() * 100 % canvas->zoom->GetRate();
	if (remainder >= 50) quotient++;
	currentVirtual.setY(quotient);

	currentReal = dynamic_cast<ZoomVisitor*>(zoomVisitor)->converter->ConvertReal(currentVirtual);

	canvas->currentX = currentReal.x();
	canvas->currentY = currentReal.y();

	Long width = canvas->currentX - canvas->startX;
	Long height = canvas->currentY - canvas->startY;
	if (width >= 150 && height >= 50) {
		canvas->templateSelected->ReSize(width, height);
	}

	canvas->repaint();
}

void DrawingTool::OnLButtonUp(DrawingPaper *canvas, QPoint point) {
	Long previousLength = canvas->flowChart->GetLength();

	canvas->indexOfSelected = canvas->flowChart->Attach(canvas->templateSelected->Clone());
	/*
	Long currentLength = dynamic_cast<FlowChart*>(canvas->flowChart)->GetLength();
	if (currentLength != previousLength && dynamic_cast<FlowChartEditor*>(canvas->parentWidget())->toolTip != NULL) {
		dynamic_cast<FlowChartEditor*>(canvas->parentWidget())->toolTip->Destroy();
		dynamic_cast<FlowChartEditor*>(canvas->parentWidget())->toolTip = NULL;
		TutorialForm *tutorialForm = (TutorialForm*)dynamic_cast<FlowChartEditor*>(canvas->parentWidget())->windows[2];
		if (tutorialForm != NULL) {
			tutorialForm->tutorialController->Update();
		}
	}
	*/

	Long(*indexes) = new Long[canvas->flowChart->GetLength()];
	indexes[0] = canvas->indexOfSelected;
	canvas->memoryController->RememberAdd(indexes, 1);

	if (canvas->templateSelected != NULL) {
		delete canvas->templateSelected;
		canvas->templateSelected = NULL;
	}
	canvas->tool = SelectingTool::Instance();
	canvas->mode = DrawingPaper::SELECT;
	canvas->repaint();

	if (indexes != 0) {
		delete[] indexes;
	}
}