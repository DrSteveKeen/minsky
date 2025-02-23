import { Component, OnDestroy, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { ElectronService, WindowUtilityService } from '@minsky/core';
import { PlotWidget } from '@minsky/shared';
import { Subject, takeUntil } from 'rxjs';

@Component({
    selector: 'minsky-plot-widget-view',
    templateUrl: './plot-widget-view.component.html',
    styleUrls: ['./plot-widget-view.component.scss'],
    standalone: true,
})
export class PlotWidgetViewComponent implements OnInit, OnDestroy {
  itemId: string;
  systemWindowId: string;

  destroy$ = new Subject<{}>();

  leftOffset = 0;
  topOffset = 0;
  height: number;
  width: number;

  constructor(
    private electronService: ElectronService,
    private route: ActivatedRoute,
    private windowUtilityService: WindowUtilityService
  ) {
    this.route.queryParams.pipe(takeUntil(this.destroy$)).subscribe((params) => {
      this.itemId = params.itemId;
      this.systemWindowId = params.systemWindowId;
    });
  }

  ngOnInit() {
    setTimeout(()=>{this.render();},10);
  }

  async render() {
    const plotCanvasContainer = document.getElementById('plot-cairo-canvas');

    const clientRect = plotCanvasContainer.getBoundingClientRect();

    this.leftOffset = Math.round(clientRect.left);

    this.topOffset = Math.round(
      await this.windowUtilityService.getElectronMenuBarHeight()
    );

    this.height = Math.round(plotCanvasContainer.clientHeight);
    this.width = Math.round(plotCanvasContainer.clientWidth);

    if (
      this.electronService.isElectron &&
      this.systemWindowId &&
      this.itemId &&
      this.height &&
      this.width
    ) {
      new PlotWidget(this.electronService.minsky.namedItems.elem(this.itemId).second)
        .renderFrame({
          parentWindowId: this.systemWindowId.toString(),
          offsetLeft: this.leftOffset,
          offsetTop: this.topOffset,
          childWidth: this.width,
          childHeight: this.height,
          scalingFactor: -1
        });
    }
  }

  ngOnDestroy() {
    this.destroy$.next(undefined);
    this.destroy$.complete();
  }
}
