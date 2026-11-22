// M4: System Analytics & Performance Metrics (Chart.js)

let responseChartInstance = null;
let bedChartInstance = null;

function initAnalyticsCharts() {
    const responseCanvas = document.getElementById('response-time-chart');
    const bedCanvas = document.getElementById('bed-utilization-chart');

    if (responseCanvas && !responseChartInstance) {
        const ctx1 = responseCanvas.getContext('2d');
        responseChartInstance = new Chart(ctx1, {
            type: 'line',
            data: {
                labels: ['08:00', '10:00', '12:00', '14:00', '16:00', '18:00', '20:00'],
                datasets: [{
                    label: 'Avg Response Time (Minutes)',
                    data: [9.2, 8.4, 11.1, 8.9, 7.8, 10.2, 8.1],
                    borderColor: '#16a34a',
                    backgroundColor: 'rgba(22, 163, 74, 0.1)',
                    fill: true,
                    tension: 0.4,
                    borderWidth: 3
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: { legend: { display: false } },
                scales: {
                    y: { beginAtZero: false, min: 5, max: 15 }
                }
            }
        });
    }

    if (bedCanvas && !bedChartInstance) {
        const ctx2 = bedCanvas.getContext('2d');
        bedChartInstance = new Chart(ctx2, {
            type: 'bar',
            data: {
                labels: ['Max', 'SMIH', 'Synergy', 'GEIMS', 'Doon', 'CMI'],
                datasets: [
                    {
                        label: 'Available ICU',
                        data: [6, 18, 9, 14, 4, 3],
                        backgroundColor: '#22c55e'
                    },
                    {
                        label: 'Occupied ICU',
                        data: [22, 102, 36, 66, 26, 17],
                        backgroundColor: '#cbd5e1'
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: { legend: { position: 'top' } },
                scales: {
                    x: { stacked: true },
                    y: { stacked: true }
                }
            }
        });
    }
}

window.initAnalyticsCharts = initAnalyticsCharts;
