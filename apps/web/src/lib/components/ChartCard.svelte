<script lang="ts">
	import {
		Chart as ChartJS,
		Title,
		Tooltip,
		Legend,
		BarElement,
		LineElement,
		PointElement,
		CategoryScale,
		LinearScale,
		BarController,
		LineController,
		Filler,
		type ChartType
	} from 'chart.js';

	ChartJS.register(
		Title,
		Tooltip,
		Legend,
		BarElement,
		LineElement,
		PointElement,
		CategoryScale,
		LinearScale,
		BarController,
		LineController,
		Filler
	);

	interface Props {
		title: string;
		data: number[];
		labels: string[];
		type?: ChartType;
		color?: string;
		bgColor?: string;
		icon?: string;
	}
	let {
		title,
		data,
		labels,
		type = 'bar',
		color = '#4a8a65',
		bgColor = 'rgba(74, 138, 101, 0.2)',
		icon = 'show_chart'
	}: Props = $props();

	let canvas: HTMLCanvasElement | undefined = $state();
	let chart: ChartJS | undefined;

	$effect(() => {
		if (!canvas) return;

		// Create gradient for line charts
		let areaBg = bgColor;
		if (type === 'line' && canvas) {
			const ctx = canvas.getContext('2d');
			if (ctx) {
				const gradient = ctx.createLinearGradient(0, 0, 0, 140);
				gradient.addColorStop(0, bgColor);
				gradient.addColorStop(1, 'rgba(255, 255, 255, 0)');
				areaBg = gradient as unknown as string;
			}
		}

		chart = new ChartJS(canvas, {
			type: type,
			data: {
				labels: $state.snapshot(labels),
				datasets: [
					{
						label: title,
						data: $state.snapshot(data),
						backgroundColor: type === 'line' ? areaBg : bgColor,
						borderColor: color,
						borderWidth: 2,
						borderRadius: type === 'bar' ? 4 : 0,
						barPercentage: type === 'bar' ? 0.7 : undefined,
						fill: type === 'line',
						tension: type === 'line' ? 0.4 : 0,
						pointRadius: type === 'line' ? 2 : 0,
						pointHoverRadius: type === 'line' ? 6 : 0,
						pointBackgroundColor: color
					}
				]
			},
			options: {
				responsive: true,
				maintainAspectRatio: false,
				interaction: {
					mode: 'index',
					intersect: false
				},
				plugins: {
					legend: { display: false },
					tooltip: {
						enabled: true,
						backgroundColor: 'rgba(20, 32, 24, 0.9)',
						titleColor: 'rgba(255, 255, 255, 0.9)',
						bodyColor: color,
						bodyFont: { weight: 'bold', size: 14 },
						padding: 12,
						cornerRadius: 8,
						displayColors: false
					}
				},
				scales: {
					y: {
						beginAtZero: false,
						grid: {
							display: true,
							color: 'rgba(150, 150, 150, 0.1)'
						},
						ticks: {
							display: true,
							color: 'rgba(150, 150, 150, 0.5)',
							font: { size: 10, family: 'Inter' },
							maxTicksLimit: 5
						},
						border: { display: false }
					},
					x: {
						grid: { display: false },
						ticks: {
							display: true,
							color: 'rgba(150, 150, 150, 0.5)',
							font: { size: 10, family: 'Inter' },
							maxTicksLimit: 6
						},
						border: { display: false }
					}
				}
			}
		});

		return () => {
			chart?.destroy();
		};
	});

	$effect(() => {
		if (chart && labels.length && data.length) {
			chart.data.labels = $state.snapshot(labels);
			chart.data.datasets[0].data = $state.snapshot(data);
			// we can update type specific props too, but simple update is fine
			chart.update();
		}
	});
</script>

<div
	class="flex flex-col gap-6 rounded-3xl border border-outline-variant bg-surface-variant p-6 shadow-sm transition-colors duration-300"
>
	<div class="flex items-start justify-between gap-4">
		<div class="flex flex-col gap-1">
			<h3 class="text-base font-bold leading-none tracking-tight text-on-surface">{title}</h3>
			<p class="text-xs leading-none text-on-surface-variant">24-hour fluctuation history</p>
		</div>
		<div class="flex h-10 w-10 items-center justify-center rounded-2xl bg-surface">
			<span class="material-symbols-outlined text-lg opacity-80" style="color: {color};"
				>{icon}</span
			>
		</div>
	</div>

	<div class="h-36 w-full">
		<canvas bind:this={canvas}></canvas>
	</div>

	<div
		class="flex justify-between text-xs font-bold uppercase tracking-widest text-on-surface-variant"
	>
		<span>24h ago</span>
		<span>Now</span>
	</div>
</div>
