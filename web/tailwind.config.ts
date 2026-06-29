import type { Config } from 'tailwindcss';

export default {
  content: ['./index.html', './src/**/*.{ts,tsx}'],
  theme: {
    extend: {
      colors: {
        // PlayStation design system — deep blue-black felt, PlayStation blue CTAs.
        canvas: '#000000',
        'canvas-dark': '#000000',
        'surface-1': '#121314', // elevated panels
        'surface-2': '#181818', // cards / pills
        'surface-3': '#191a1b',

        // Hairlines — faint cool white.
        hairline: 'rgba(229,229,229,0.2)',
        'hairline-strong': 'rgba(229,229,229,0.32)',
        'hairline-tertiary': 'rgba(229,229,229,0.45)',

        // Ink — on dark.
        ink: '#ffffff',
        'ink-muted': '#cccccc',
        'ink-subtle': 'rgba(255,255,255,0.7)',
        'ink-tertiary': 'rgba(229,229,229,0.55)',

        // Primary CTA — PlayStation blue.
        primary: { DEFAULT: '#0070d1', hover: '#0064b7', focus: '#004d8d' },
        'primary-dark': '#004d8d',
        'on-primary': '#ffffff',

        // Warm commerce accent (scarce) + game accents.
        commerce: '#d53b00',
        success: '#53b1ff',
        danger: '#d53b00',
        landlord: '#ffce21',
        'landlord-soft': 'rgba(255,206,33,0.12)',
        peasant: '#53b1ff',
        'peasant-soft': 'rgba(83,177,255,0.12)',

        // Felt table — blue-black.
        felt: {
          DEFAULT: '#0a1018',
          light: '#16243b',
          dark: '#000000',
          edge: 'rgba(229,229,229,0.2)',
        },

        // Gold trim.
        'gold-a': '#ffce21',
        'gold-b': '#f5a623',
        'gold-c': '#ee8e00',

        // Playing-card faces stay crisp white.
        'card-face': '#ffffff',
        'card-face-edge': 'rgba(255,255,255,0.12)',
        'card-ink': '#1a1a1a',
        'card-red': '#d53b00',
      },
      fontFamily: {
        display: ['Inter', 'PingFang SC', 'Microsoft YaHei', 'system-ui', 'sans-serif'],
        sans: ['Inter', 'PingFang SC', 'Microsoft YaHei', 'system-ui', 'sans-serif'],
        mono: ['ui-monospace', 'SFMono-Regular', 'Menlo', 'monospace'],
      },
      borderRadius: {
        xs: '4px',
        sm: '8px',
        md: '12px',
        lg: '16px',
        xl: '20px',
        xxl: '28px',
        pill: '9999px',
      },
      spacing: { xxs: '4px', section: '96px' },
      boxShadow: {
        card: '0 6px 16px rgba(0,0,0,0.45)',
        'card-lift': '0 12px 26px rgba(0,112,209,0.4)',
        fan: '0 2px 6px rgba(0,0,0,0.5)',
        overlay: '0 24px 60px rgba(0,0,0,0.6)',
        'glow-primary': '0 0 0 3px var(--tw-shadow-color, #0070d1), 0 12px 26px rgba(0,112,209,0.4)',
        'glow-landlord': '0 0 16px rgba(255,206,33,0.4)',
        'seat-active': '0 0 0 4px rgba(0,112,209,0.25), 0 0 24px rgba(0,112,209,0.45)',
        inset: 'inset 0 1px 0 rgba(255,255,255,0.06)',
      },
      keyframes: {
        'fade-in': {
          from: { opacity: '0' },
          to: { opacity: '1' },
        },
        'scale-in': {
          from: { opacity: '0', transform: 'scale(0.95) translateY(8px)' },
          to: { opacity: '1', transform: 'scale(1) translateY(0)' },
        },
        pop: {
          from: { opacity: '0', transform: 'translateY(8px) scale(0.92)' },
          to: { opacity: '1', transform: 'none' },
        },
        deal: {
          from: { opacity: '0', transform: 'translateY(40px) rotate(-8deg)' },
          to: { opacity: '1', transform: 'none' },
        },
        'toast-in': {
          from: { opacity: '0', transform: 'translate(-50%, -50%) scale(0.9)' },
          to: { opacity: '1', transform: 'translate(-50%, -50%) scale(1)' },
        },
        'pulse-ring': {
          '0%, 100%': { boxShadow: '0 0 0 0 rgba(0,112,209,0)' },
          '50%': { boxShadow: '0 0 0 5px rgba(0,112,209,0.25)' },
        },
        'pulse-soft': {
          '0%, 100%': { opacity: '0.4' },
          '50%': { opacity: '1' },
        },
        float: {
          '0%, 100%': { transform: 'translateY(0)' },
          '50%': { transform: 'translateY(-6px)' },
        },
      },
      animation: {
        'fade-in': 'fade-in 0.3s ease both',
        'scale-in': 'scale-in 0.24s cubic-bezier(0.16,1,0.3,1) both',
        pop: 'pop 0.28s cubic-bezier(0.2,0.9,0.3,1.4) both',
        deal: 'deal 0.45s cubic-bezier(0.2,0.85,0.3,1.1) both',
        'toast-in': 'toast-in 0.18s ease-out both',
        'pulse-ring': 'pulse-ring 2.2s ease-in-out infinite',
        'pulse-soft': 'pulse-soft 1.3s ease-in-out infinite',
        float: 'float 4s ease-in-out infinite',
      },
    },
  },
  plugins: [],
} satisfies Config;
