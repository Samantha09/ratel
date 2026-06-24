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
      },
      fontFamily: {
        display: ['Inter', 'SF Pro Display', 'system-ui', 'sans-serif'],
        sans: ['Inter', 'SF Pro Text', 'system-ui', 'sans-serif'],
        mono: ['ui-monospace', 'SFMono-Regular', 'Menlo', 'monospace'],
      },
      borderRadius: { xs: '4px', sm: '6px', md: '8px', lg: '12px', xl: '16px' },
      spacing: { xxs: '4px', section: '96px' },
    },
  },
  plugins: [],
} satisfies Config;
