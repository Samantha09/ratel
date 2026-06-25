import type { Config } from 'tailwindcss';

export default {
  content: ['./index.html', './src/**/*.{ts,tsx}'],
  theme: {
    extend: {
      colors: {
        canvas: '#010102',
        'surface-1': '#0f1011',
        'surface-2': '#141516',
        'surface-3': '#18191a',
        'surface-4': '#191a1b',
        hairline: '#23252a',
        'hairline-strong': '#34343a',
        'hairline-tertiary': '#3e3e44',
        ink: '#f7f8f8',
        'ink-muted': '#d0d6e0',
        'ink-subtle': '#8a8f98',
        'ink-tertiary': '#62666d',
        primary: { DEFAULT: '#5e6ad2', hover: '#828fff', focus: '#5e69d1' },
        'on-primary': '#ffffff',
        success: '#27a644',
        danger: '#e5484d',
        landlord: '#d8a657',
        // Playing-card faces use the inverse surface ladder from DESIGN.md so the
        // deck reads as physical cards against the near-black canvas.
        'card-face': '#f6f7f7',
        'card-face-edge': '#d9dbdd',
        'card-ink': '#15171a',
        'card-red': '#d83a3f',
      },
      fontFamily: {
        display: ['Inter', 'SF Pro Display', 'system-ui', 'sans-serif'],
        sans: ['Inter', 'SF Pro Text', 'system-ui', 'sans-serif'],
        mono: ['ui-monospace', 'SFMono-Regular', 'Menlo', 'monospace'],
      },
      borderRadius: { xs: '4px', sm: '6px', md: '8px', lg: '12px', xl: '16px', xxl: '24px', pill: '9999px' },
      spacing: { xxs: '4px', section: '96px' },
      boxShadow: {
        card: '0 1px 2px rgba(0,0,0,0.5), 0 6px 16px rgba(0,0,0,0.45)',
        'card-lift': '0 8px 24px rgba(0,0,0,0.55), 0 2px 6px rgba(0,0,0,0.5)',
        // Left-cast shadow so overlapping fanned cards stay visually separated.
        fan: '-4px 0 7px -2px rgba(0,0,0,0.45), 0 5px 14px rgba(0,0,0,0.4)',
        overlay: '0 24px 64px rgba(0,0,0,0.6)',
        'glow-primary': '0 0 0 1px rgba(94,106,210,0.6), 0 0 24px rgba(94,106,210,0.35)',
      },
      keyframes: {
        'fade-in': {
          from: { opacity: '0' },
          to: { opacity: '1' },
        },
        'scale-in': {
          from: { opacity: '0', transform: 'scale(0.96) translateY(6px)' },
          to: { opacity: '1', transform: 'scale(1) translateY(0)' },
        },
        'deal-in': {
          from: { opacity: '0', transform: 'translateY(14px) rotate(-2deg)' },
          to: { opacity: '1', transform: 'translateY(0) rotate(0)' },
        },
        'toast-in': {
          from: { opacity: '0', transform: 'translate(-50%, 8px)' },
          to: { opacity: '1', transform: 'translate(-50%, 0)' },
        },
        'pulse-ring': {
          '0%, 100%': { boxShadow: '0 0 0 0 rgba(94,106,210,0.0)' },
          '50%': { boxShadow: '0 0 0 4px rgba(94,106,210,0.35)' },
        },
      },
      animation: {
        'fade-in': 'fade-in 0.25s ease-out both',
        'scale-in': 'scale-in 0.22s cubic-bezier(0.16,1,0.3,1) both',
        'deal-in': 'deal-in 0.32s cubic-bezier(0.16,1,0.3,1) both',
        'toast-in': 'toast-in 0.2s ease-out both',
        'pulse-ring': 'pulse-ring 2s ease-in-out infinite',
      },
    },
  },
  plugins: [],
} satisfies Config;
